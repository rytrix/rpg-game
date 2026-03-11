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

    const aiScene* scene = m_importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
    m_directory = m_directory.substr(0, m_directory.find_last_of('/'));

    util_assert(scene->mRootNode != nullptr, "Model::Model: Root node is nullptr");
    process_node(scene->mRootNode, scene);

    usize offset = 0;
    for (usize i = 0; i < m_mesh.m_base_vertices.size(); i++) {
        m_mesh.m_base_vertices.at(i).m_offset = offset;
        offset += m_mesh.m_base_vertices.at(i).m_count;
    }

    // for (auto& bone : m_mesh.m_bone_id_map) {
    //     std::println("Bone Name: \"{}\", ID: {}", bone.first, bone.second);
    // }

    // process_animations
    for (u32 i = 0; i < scene->mNumAnimations; i++) {
        auto* animation = scene->mAnimations[i];
        LOG_INFO(std::format("Animation info: Name: {}, Duration: {}", animation->mName.C_Str(), animation->mDuration));
        m_mesh.m_total_animation_time = animation->mDuration;
        for (u32 j = 0; j < animation->mNumChannels; j++) {
            auto* bone_animation = animation->mChannels[j];
            if (m_mesh.m_bone_id_map.contains(bone_animation->mNodeName.C_Str())) {
                u32 value = m_mesh.m_bone_id_map.at(bone_animation->mNodeName.C_Str());
                LOG_DEBUG(std::format("Node name: \"{}\", index: {}", bone_animation->mNodeName.C_Str(), value));
                m_mesh.m_bones.at(value).m_animation.init(bone_animation);
            }
        }
        break;
    }
    // process_animations end

    m_mesh.setup_mesh();

    initialized = true;
}

Model::~Model()
{
    initialized = false;
}

void Model::draw_untextured(ShaderProgram& shader, const std::span<glm::mat4> model)
{
    util_assert(initialized == true, "Model has not been initialized");

    // shader.set_mat4("model", model[0]);
    m_mesh.update_model_ssbos(model);
    m_mesh.draw();
}

void Model::draw(ShaderProgram& shader, const std::span<glm::mat4> model)
{
    util_assert(initialized == true, "Model has not been initialized");

    // shader.set_mat4("model", model[0]);
    m_mesh.update_model_ssbos(model);
    static float animation_time = 0.0F;
    animation_time += 0.1F;
    std::println("animation time: {}", animation_time);
    if (m_mesh.m_has_bones) {
        m_mesh.update_bone_matrices(animation_time);
    }
    m_mesh.draw(shader);
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
    auto base_vertex = static_cast<GLsizei>(m_mesh.m_vertices.size());
    auto count = static_cast<GLsizei>(m_mesh.m_indices.size());
    auto base_bone = m_mesh.m_bones.size();

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

        m_mesh.m_vertices.push_back(vertex);
    }

    for (u32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (u32 j = 0; j < face.mNumIndices; j++) {
            m_mesh.m_indices.push_back(face.mIndices[j]);
        }
    }

    if (scene->HasMaterials()) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        Texture* diffuse_map = load_material_texture(material, aiTextureType_DIFFUSE);
        if (diffuse_map == nullptr) {
            LOG_WARN("Using default albedo texture map");
            m_mesh.m_diffuse_textures.push_back(get_placeholder_texture_albedo());
        } else {
            m_mesh.m_diffuse_textures.push_back(diffuse_map);
        }

        Texture* metallic_roughness_map = load_material_texture(material, aiTextureType_GLTF_METALLIC_ROUGHNESS);
        if (metallic_roughness_map == nullptr) {
            LOG_WARN("Using default metallic texture map");
            m_mesh.m_metallic_roughness_textures.push_back(get_placeholder_texture_metallic());
        } else {
            m_mesh.m_metallic_roughness_textures.push_back(metallic_roughness_map);
        }

        Texture* normal_map = load_material_texture(material, aiTextureType_NORMALS);
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
        m_mesh.m_vertex_bones.resize(m_mesh.m_vertex_bones.size() + mesh->mNumVertices);
        // std::println("vertex bones size {}", m_mesh.m_vertex_bones.size());
        for (u32 i = 0; i < mesh->mNumBones; i++) {
            // Each bone has mName, mNumWeights, mOffsetMatrix, mWeights

            // Reverse map each vertex to the bones that influence it (to be accessed in the vertex shader)
            // preferrably as a seperate data structure

            // Each bone has a ID (and its mat4 transformation)
            // Each vertex has a list of IDs and Weights that correspond to those bones

            // Also all of these bones have to be kept inside of each base vertex...
            aiBone* bone = mesh->mBones[i];

            m_mesh.m_bone_id_map[bone->mName.C_Str()] = i;

            glm::mat4 offset_matrix = mat4_to_mat4(bone->mOffsetMatrix);

            m_mesh.m_bones.emplace_back(offset_matrix, nullptr);

            for (u32 j = 0; j < bone->mNumWeights; j++) {
                const aiVertexWeight vw = bone->mWeights[j];
                m_mesh.m_vertex_bones
                    .at(base_vertex + vw.mVertexId)
                    .add_bone(i, vw.mWeight);
            }
        }

        // LOG_INFO(std::format("mesh->mNumAnimMeshes {}", mesh->mNumAnimMeshes));
        // for (u32 i = 0; i < mesh->mNumAnimMeshes; i++) {
        //     auto* animation = mesh->mAnimMeshes[i];
        //     // animation->mName;
        //     std::println("aiAnimMesh Name: {}", animation->mName.C_Str());
        // }
    }

    count = static_cast<GLsizei>(m_mesh.m_indices.size()) - count;

    m_mesh.m_base_vertices.emplace_back(
        count,
        base_vertex,
        base_bone);
}

Texture* Model::load_material_texture(aiMaterial* mat, aiTextureType type)
{
    if (mat->GetTextureCount(type) > 0) {
        aiString str;
        mat->GetTexture(type, 0, &str);
        std::string texture_path = (m_directory + "/" + str.C_Str());

        TextureInfo texture_info;
        texture_info.from_file = GL_TRUE;
        texture_info.min_filter = GL_LINEAR_MIPMAP_LINEAR;
        texture_info.mag_filter = GL_LINEAR;
        texture_info.mipmaps = true;
        texture_info.mipmap_levels = 0;
        texture_info.file_path = texture_path.c_str();
        texture_info.flip = false;

        LOG_INFO(std::format("Loading {} type {}", texture_path, aiTextureTypeToString(type)));

        Texture& texture = m_texture_cache.get_or_create(texture_path, texture_info);
        texture.set_max_anisotropy(16.0F);
        return &texture;
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
