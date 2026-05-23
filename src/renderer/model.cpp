#include "model.hpp"

#include "assimp/Importer.hpp"
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>

#include "../utils/helpers.hpp"

namespace Renderer {

Model::Model(const char* file_path, GlobalAppData* app_data)
{
    init(file_path, app_data);
}

void Model::init(const char* file_path, GlobalAppData* app_data)
{
    util_assert(initialized == false, "already initialized");

    m_directory = file_path;
    m_app_data = app_data;

    util_assert(std::filesystem::exists(file_path), std::format("Model \"{}\" is an invalid path", file_path));
    m_directory = m_directory.substr(0, m_directory.find_last_of('/'));

    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    const aiScene* scene = importer.ReadFile(file_path,
        aiProcess_Triangulate
            | aiProcess_FlipUVs
            | aiProcess_CalcTangentSpace
            | aiProcess_JoinIdenticalVertices);

    setup_mesh(scene);
    setup_animations(scene);

    initialized = true;
}

void Model::setup_mesh(const aiScene* scene)
{
    m_mesh.m_app_data = m_app_data;

    process_node(scene->mRootNode, scene);

    usize offset = 0;
    for (usize i = 0; i < m_mesh.m_base_vertices.size(); i++) {
        m_mesh.m_base_vertices.at(i).m_offset = offset;
        offset += m_mesh.m_base_vertices.at(i).m_count;
    }

    m_mesh.setup_mesh();
}

void Model::setup_animations(const aiScene* scene)
{
    glm::mat4 global_inverse_transform = glm::inverse(mat4_to_mat4(scene->mRootNode->mTransformation));

    m_mesh.m_animations.resize(scene->mNumAnimations);

    for (u32 i = 0; i < scene->mNumAnimations; i++) {
        auto* animation = scene->mAnimations[i];
        LOG_INFO(std::format("Animation info: Name: {}, Duration: {}, Ticks Per Second: {}", animation->mName.C_Str(), animation->mDuration, animation->mTicksPerSecond));

        m_mesh.m_animations[i].init(scene,
            animation,
            m_mesh.m_bone_id_map,
            global_inverse_transform);
    }
}

Model::~Model()
{
    initialized = false;
}

void Model::draw_untextured(Shader& shader)
{
    util_assert(initialized == true, "not initialized");

    m_mesh.draw_untextured(shader);
}

void Model::draw(Shader& shader)
{
    util_assert(initialized == true, "not initialized");

    m_mesh.draw(shader);
}

void Model::update(std::span<glm::mat4> models, std::span<AnimationData*> animation_data)
{
    util_assert(initialized == true, "not initialized");
    m_mesh.next_ssbo_frame();
    m_mesh.update_instance_count(models.size());
    m_mesh.update_model_ssbos(models);
    if (m_mesh.m_has_bones) {
        m_mesh.update_bone_matrices(animation_data);
    }
}

Mesh* Model::get_mesh()
{
    util_assert(initialized == true, "not initialized");
    return &m_mesh;
}

void Model::process_node(aiNode* node, const aiScene* scene)
{
    for (u32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        LOG_DEBUG(std::format("Loading mesh: {} from scene", node->mMeshes[i]));
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
        Mesh::Vertex& vertex = m_mesh.m_vertex_data.m_vertices.emplace_back();

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
    }

    for (u32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (u32 j = 0; j < face.mNumIndices; j++) {
            m_mesh.m_vertex_data.m_indices.push_back(face.mIndices[j]);
        }
    }

    if (scene->HasMaterials()) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        auto diffuse_map = load_material_texture(material, aiTextureType_DIFFUSE, scene);
        if (diffuse_map.generation.valid == 0) {
            LOG_WARN("Using default albedo texture map");
            m_mesh.m_diffuse_textures.push_back(m_app_data->m_default_textures.get_albedo());
        } else {
            m_mesh.m_diffuse_textures.push_back(diffuse_map);
        }

        auto metallic_roughness_map = load_material_texture(material, aiTextureType_GLTF_METALLIC_ROUGHNESS, scene);
        if (metallic_roughness_map.generation.valid == 0) {
            LOG_WARN("Using default metallic texture map");
            m_mesh.m_metallic_roughness_textures.push_back(m_app_data->m_default_textures.get_metallic());
        } else {
            m_mesh.m_metallic_roughness_textures.push_back(metallic_roughness_map);
        }

        auto normal_map = load_material_texture(material, aiTextureType_NORMALS, scene);
        if (normal_map.generation.valid == 0) {
            LOG_WARN("Using default normal texture map");
            m_mesh.m_normal_textures.push_back(m_app_data->m_default_textures.get_normal());
        } else {
            m_mesh.m_normal_textures.push_back(normal_map);
        }
    }

    if (mesh->HasBones()) {
        m_mesh.m_has_bones = true;
        m_mesh.m_vertex_data.m_bones.resize(m_mesh.m_vertex_data.m_bones.size() + mesh->mNumVertices);

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

    AABB& aabb = m_mesh.m_aabbs.emplace_back();
    aabb.min.x = mesh->mAABB.mMin.x;
    aabb.min.y = mesh->mAABB.mMin.y;
    aabb.min.z = mesh->mAABB.mMin.z;

    aabb.max.x = mesh->mAABB.mMax.x;
    aabb.max.y = mesh->mAABB.mMax.y;
    aabb.max.z = mesh->mAABB.mMax.z;

    count = static_cast<GLsizei>(m_mesh.m_vertex_data.m_indices.size()) - count;

    m_mesh.m_base_vertices.emplace_back(
        count,
        base_vertex);
}

Handle Model::load_material_texture(const aiMaterial* mat, const aiTextureType type, const aiScene* scene)
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

        auto* texture_cache = &m_app_data->m_texture_cache;
        if (embedded_texture == nullptr) {
            Utils::String texture_path;
            texture_path.format("{}/{}", m_directory.c_str(), str.C_Str());

            texture_info.origin = TextureOrigin::File;
            texture_info.file_path = texture_path.c_str();

            LOG_INFO(std::format("Loading {} type {}", texture_path.view(), aiTextureTypeToString(type)));

            Handle handle;
            if (texture_cache->contains(texture_path)) {
                handle = texture_cache->get(texture_path);
            } else {
                handle = texture_cache->get_or_create(texture_path, texture_info);
                auto* texture = texture_cache->get(handle);
                texture->set_max_anisotropy(16.0F);
            }
            return handle;
        } else {
            Utils::String texture_path(str.C_Str());

            LOG_INFO(std::format("Loading {} type {}", texture_path.view(), aiTextureTypeToString(type)));
            if (texture_cache->contains(texture_path)) {
                return texture_cache->get(texture_path);
            } else {
                texture_info.origin = TextureOrigin::Memory;
                texture_info.memory = (char*)embedded_texture->pcData;
                texture_info.memory_size = embedded_texture->mWidth;

                auto handle = texture_cache->get_or_create(texture_path, texture_info);
                Texture* texture = texture_cache->get(handle);
                texture->set_max_anisotropy(16.0F);

                // int width, height, channels;
                // TextureSubimageInfo subimage_info;
                // stbi_set_flip_vertically_on_load((int)texture_info.flip);
                // unsigned char* data = stbi_load_from_memory((const stbi_uc*)embedded_texture->pcData, embedded_texture->mWidth, &width, &height, &channels, 0);

                // texture_info.from_file = GL_FALSE;
                // if (channels == 4) {
                //     texture_info.internal_format = GL_RGBA8;
                //     subimage_info.format = GL_RGBA;
                // } else if (channels == 3) {
                //     texture_info.internal_format = GL_RGB8;
                //     subimage_info.format = GL_RGB;
                // }
                // texture_info.size.width = width;
                // texture_info.size.height = height;
                // texture_info.size.depth = 0;
                // auto handle = texture_cache->get_or_create(texture_path, texture_info);
                // Texture* texture = texture_cache->get(handle);

                // subimage_info.pixels = data;
                // subimage_info.size = texture_info.size;
                // subimage_info.type = GL_UNSIGNED_BYTE;
                // texture->sub_image(subimage_info);

                // texture->set_max_anisotropy(16.0F);

                // stbi_image_free(data);

                return handle;
            }
        }
    } else {
        return { .generation = { .valid = 0, .id = 0 }, .index = 0 };
    }
}

} // namespace Renderer
