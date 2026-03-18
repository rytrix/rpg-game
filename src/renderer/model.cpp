#include "model.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>

#include "../utils/helpers.hpp"

namespace {

} // anonymous namespace

namespace Renderer {

Model::Model(const char* file_path)
{
    init(file_path);
}

void Model::init(const char* file_path)
{
    util_assert(initialized == false, "Model::init() has already been initialized");

    m_directory = file_path;

    util_assert(std::filesystem::exists(file_path), std::format("Model \"{}\" is an invalid path", file_path));

    Assimp::Importer importer;
    // importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 10.0F);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    const aiScene* scene = importer.ReadFile(file_path,
        aiProcess_Triangulate
            | aiProcess_FlipUVs
            | aiProcess_CalcTangentSpace
            | aiProcess_JoinIdenticalVertices);
    m_directory = m_directory.substr(0, m_directory.find_last_of('/'));

    util_assert(scene->mRootNode != nullptr, "Model::Model: Root node is nullptr");

    glm::mat4 global_inverse_transform = glm::inverse(mat4_to_mat4(scene->mRootNode->mTransformation));

    process_node(scene->mRootNode, scene);

    usize offset = 0;
    for (usize i = 0; i < m_mesh.m_base_vertices.size(); i++) {
        m_mesh.m_base_vertices.at(i).m_offset = offset;
        offset += m_mesh.m_base_vertices.at(i).m_count;
    }

    m_animations.resize(scene->mNumAnimations);

    for (u32 i = 0; i < scene->mNumAnimations; i++) {
        auto* animation = scene->mAnimations[i];
        LOG_INFO(std::format("Animation info: Name: {}, Duration: {}, Ticks Per Second: {}", animation->mName.C_Str(), animation->mDuration, animation->mTicksPerSecond));

        m_animations[i].init(scene,
            animation,
            m_mesh.m_bone_id_map,
            global_inverse_transform,
            static_cast<float>(animation->mDuration),
            static_cast<float>(animation->mTicksPerSecond));
    }
    if (m_animations.size() >= 1) {
        set_animation(0);
    }

    m_mesh.setup_mesh();

    initialized = true;
}

Model::~Model()
{
    initialized = false;
}

void Model::draw_untextured(ShaderProgram& shader)
{
    util_assert(initialized == true, "Model has not been initialized");

    // shader.set_mat4("model", model[0]);
    m_mesh.draw_untextured(shader);
}

void Model::draw(ShaderProgram& shader)
{
    util_assert(initialized == true, "Model has not been initialized");

    // shader.set_mat4("model", model[0]);
    m_mesh.draw(shader);
}

void Model::update(std::span<glm::mat4> models, float animation_time)
{
    m_mesh.update_model_ssbos(models);
    if (m_mesh.m_has_bones) {
        m_mesh.update_bone_matrices(animation_time);
    }
}

const Mesh* Model::get_mesh()
{
    util_assert(initialized == true, "Model has not been initialized");
    return &m_mesh;
}

bool Model::has_bones() const
{
    return m_mesh.m_has_bones;
}

std::deque<Animation>& Model::get_animations()
{
    return m_animations;
}

u32 Model::get_current_animation() const
{
    return m_current_animation;
}

void Model::set_animation(u32 value)
{
    if (value < m_animations.size()) {
        m_mesh.m_animation = &m_animations[value];
        m_current_animation = value;
    }
}

void Model::process_node(aiNode* node, const aiScene* scene)
{
    for (u32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh, scene);
    }

    for (u32 i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}

void Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    auto base_vertex = static_cast<GLsizei>(m_mesh.m_vertex_data.m_vertices.size());
    auto count = static_cast<GLsizei>(m_mesh.m_vertex_data.m_indices.size());

    for (u32 i = 0; i < mesh->mNumVertices; i++) {
        Mesh::Vertex vertex {};
        vertex.m_pos.x = mesh->mVertices[i].x;
        vertex.m_pos.y = mesh->mVertices[i].y;
        vertex.m_pos.z = mesh->mVertices[i].z;

        vertex.m_norm.x = mesh->mNormals[i].x;
        vertex.m_norm.y = mesh->mNormals[i].y;
        vertex.m_norm.z = mesh->mNormals[i].z;

        vertex.m_tang.x = mesh->mTangents[i].x;
        vertex.m_tang.y = mesh->mTangents[i].y;
        vertex.m_tang.z = mesh->mTangents[i].z;

        if (mesh->HasTextureCoords(0)) {
            vertex.m_tex.x = mesh->mTextureCoords[0][i].x;
            vertex.m_tex.y = mesh->mTextureCoords[0][i].y;
        }

        m_mesh.m_vertex_data.m_vertices.push_back(vertex);
    }

    for (u32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (u32 j = 0; j < face.mNumIndices; j++) {
            m_mesh.m_vertex_data.m_indices.push_back(face.mIndices[j]);
        }
    }

    if (scene->HasMaterials()) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        Texture* diffuse_map = load_material_texture(material, aiTextureType_DIFFUSE, scene);
        if (diffuse_map == nullptr) {
            LOG_WARN("Using default albedo texture map");
            m_mesh.m_diffuse_textures.push_back(get_placeholder_texture_albedo());
        } else {
            m_mesh.m_diffuse_textures.push_back(diffuse_map);
        }

        Texture* metallic_roughness_map = load_material_texture(material, aiTextureType_GLTF_METALLIC_ROUGHNESS, scene);
        if (metallic_roughness_map == nullptr) {
            LOG_WARN("Using default metallic texture map");
            m_mesh.m_metallic_roughness_textures.push_back(get_placeholder_texture_metallic());
        } else {
            m_mesh.m_metallic_roughness_textures.push_back(metallic_roughness_map);
        }

        Texture* normal_map = load_material_texture(material, aiTextureType_NORMALS, scene);
        if (normal_map == nullptr) {
            LOG_WARN("Using default normal texture map");
            m_mesh.m_normal_textures.push_back(get_placeholder_texture_normal());
        } else {
            m_mesh.m_normal_textures.push_back(normal_map);
        }

        // Texture* ao_map = load_material_textures(material, aiTextureType_AMBIENT_OCCLUSION);
        // m_mesh.m_textures.push_back(ao_map);
    }

    if (mesh->HasBones()) {
        m_mesh.m_has_bones = true;
        m_mesh.m_vertex_data.m_bones.resize(m_mesh.m_vertex_data.m_bones.size() + mesh->mNumVertices);
        std::println("mesh has bones");

        for (u32 i = 0; i < mesh->mNumBones; i++) {
            // Each bone has mName, mNumWeights, mOffsetMatrix, mWeights

            // Reverse map each vertex to the bones that influence it (to be accessed in the vertex shader)
            // preferrably as a seperate data structure

            // Each bone has a ID (and its mat4 transformation)
            // Each vertex has a list of IDs and Weights that correspond to those bones

            aiBone* bone = mesh->mBones[i];

            usize bone_id;

            // There might be multiple meshes with the same bones
            if (!m_mesh.m_bone_id_map.contains(bone->mName.C_Str())) {
                bone_id = m_mesh.m_bone_id_map.size();
                m_mesh.m_bone_id_map[bone->mName.C_Str()] = bone_id;

                LOG_DEBUG(std::format("Loaded bone: \"{}\" at index {}", bone->mName.C_Str(), i));
            } else {
                bone_id = m_mesh.m_bone_id_map[bone->mName.C_Str()];
            }

            for (u32 j = 0; j < bone->mNumWeights; j++) {
                const aiVertexWeight vw = bone->mWeights[j];
                m_mesh.m_vertex_data.m_bones
                    .at(base_vertex + vw.mVertexId)
                    .add_bone(bone_id, vw.mWeight);
            }
        }
    }

    count = static_cast<GLsizei>(m_mesh.m_vertex_data.m_indices.size()) - count;

    m_mesh.m_base_vertices.emplace_back(
        count,
        base_vertex);
}

Texture* Model::load_material_texture(const aiMaterial* mat, const aiTextureType type, const aiScene* scene)
{
    if (mat->GetTextureCount(type) > 0) {
        aiString str;
        mat->GetTexture(type, 0, &str);

        const aiTexture* embedded_texture = scene->GetEmbeddedTexture(str.C_Str());

        TextureInfo texture_info;
        texture_info.min_filter = GL_LINEAR_MIPMAP_LINEAR;
        texture_info.mag_filter = GL_LINEAR;
        texture_info.mipmaps = true;
        texture_info.mipmap_levels = 0;
        texture_info.flip = false;

        if (embedded_texture == nullptr) {
            std::string texture_path = (m_directory + "/" + str.C_Str());
            texture_info.from_file = GL_TRUE;
            texture_info.file_path = texture_path.c_str();

            LOG_INFO(std::format("Loading {} type {}", texture_path, aiTextureTypeToString(type)));

            Texture& texture = m_texture_cache.get_or_create(texture_path, texture_info);
            texture.set_max_anisotropy(16.0F);
            return &texture;
        } else {
            std::string texture_path = str.C_Str();

            LOG_INFO(std::format("Loading {} type {}", texture_path, aiTextureTypeToString(type)));
            if (m_texture_cache.contains(texture_path)) {
                Texture& texture = m_texture_cache.get_or_create(texture_path, texture_info);
                return &texture;
            } else {
                int width, height, channels;
                TextureSubimageInfo subimage_info;
                stbi_set_flip_vertically_on_load((int)texture_info.flip);
                unsigned char* data = stbi_load_from_memory((const stbi_uc*)embedded_texture->pcData, embedded_texture->mWidth, &width, &height, &channels, 0);

                texture_info.from_file = GL_FALSE;
                if (channels == 4) {
                    texture_info.internal_format = GL_RGBA8;
                    subimage_info.format = GL_RGBA;
                } else if (channels == 3) {
                    texture_info.internal_format = GL_RGB8;
                    subimage_info.format = GL_RGB;
                }
                texture_info.size.width = width;
                texture_info.size.height = height;
                texture_info.size.depth = 0;
                Texture& texture = m_texture_cache.get_or_create(texture_path, texture_info);

                subimage_info.pixels = data;
                subimage_info.size = texture_info.size;
                subimage_info.type = GL_UNSIGNED_BYTE;
                texture.sub_image(subimage_info);
                texture.set_max_anisotropy(16.0F);

                stbi_image_free(data);

                return &texture;
            }
        }
    } else {
        return nullptr;
    }
}

namespace {
    Texture* placeholder_texture_albedo = nullptr;
    Texture* placeholder_texture_metallic = nullptr;
    Texture* placeholder_texture_normal = nullptr;
}

void Model::init_placeholder_textures()
{
    TextureSize size = {
        .width = 1,
        .height = 1,
        .depth = 0,
    };
    TextureInfo texture_info;
    texture_info.size = size;
    texture_info.from_file = GL_FALSE;
    texture_info.mipmaps = false;
    texture_info.flip = false;
    texture_info.internal_format = GL_RGBA8;

    TextureSubimageInfo subimage_info;
    subimage_info.size = size;
    subimage_info.format = GL_RGBA;
    subimage_info.type = GL_UNSIGNED_BYTE;

    std::array<u8, 4> data_albedo = { 255, 255, 255, 255 };
    subimage_info.pixels = data_albedo.data();
    placeholder_texture_albedo = new Texture(texture_info);
    placeholder_texture_albedo->sub_image(subimage_info);

    std::array<u8, 4> data_metallic = { 255, 0, 0, 0 };
    subimage_info.pixels = data_metallic.data();
    placeholder_texture_metallic = new Texture(texture_info);
    placeholder_texture_metallic->sub_image(subimage_info);

    subimage_info.format = GL_RGBA;
    subimage_info.type = GL_FLOAT;

    std::array<float, 4> data_normal = { 0.5F, 0.5F, 1.0F, 0.0F };
    subimage_info.pixels = data_normal.data();
    placeholder_texture_normal = new Texture(texture_info);
    placeholder_texture_normal->sub_image(subimage_info);
}

void Model::destroy_placeholder_textures()
{
    delete placeholder_texture_albedo;
    delete placeholder_texture_metallic;
    delete placeholder_texture_normal;
    placeholder_texture_albedo = nullptr;
    placeholder_texture_metallic = nullptr;
    placeholder_texture_normal = nullptr;
}

Texture* Model::get_placeholder_texture_albedo()
{
    if (placeholder_texture_albedo == nullptr) {
        init_placeholder_textures();
    }
    return placeholder_texture_albedo;
}

Texture* Model::get_placeholder_texture_normal()
{
    if (placeholder_texture_normal == nullptr) {
        init_placeholder_textures();
    }
    return placeholder_texture_normal;
}

Texture* Model::get_placeholder_texture_metallic()
{
    if (placeholder_texture_metallic == nullptr) {
        init_placeholder_textures();
    }
    return placeholder_texture_metallic;
}

} // namespace Renderer
