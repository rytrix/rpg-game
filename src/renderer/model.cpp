#include "model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>

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
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    m_directory = m_directory.substr(0, m_directory.find_last_of('/'));

    util_assert(scene->mRootNode != nullptr, "Model::Model: Root node is nullptr");
    process_node(scene->mRootNode, scene);

    usize offset = 0;
    for (usize i = 0; i < m_mesh.m_base_vertices.size(); i++) {
        m_mesh.m_base_vertices.at(i).m_offset = offset;
        offset += m_mesh.m_base_vertices.at(i).m_count;
    }
    m_mesh.setup_mesh();

    initialized = true;
}

Model::~Model()
{
    initialized = false;
}

void Model::draw_untextured(ShaderProgram& shader, const std::vector<glm::mat4>& model)
{
    util_assert(initialized == true, "Model has not been initialized");

    // shader.set_mat4("model", model[0]);
    m_mesh.update_model_ssbos(model);
    m_mesh.draw();
}

void Model::draw(ShaderProgram& shader, const std::vector<glm::mat4>& model)
{
    util_assert(initialized == true, "Model has not been initialized");

    // shader.set_mat4("model", model[0]);
    m_mesh.update_model_ssbos(model);
    m_mesh.draw(shader);
}

const Mesh* Model::get_mesh()
{
    util_assert(initialized == true, "Model has not been initialized");
    return &m_mesh;
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

    count = static_cast<GLsizei>(m_mesh.m_indices.size()) - count;

    m_mesh.m_base_vertices.emplace_back(
        count,
        base_vertex);
}

Texture* Model::load_material_texture(aiMaterial* mat, aiTextureType type)
{
    if (mat->GetTextureCount(type) > 0) {
        aiString str;
        mat->GetTexture(type, 0, &str);
        std::string texture_path = (m_directory + "/" + str.C_Str());

        TextureInfo texture_info;
        texture_info.from_file = GL_TRUE;
        texture_info.file_path = texture_path.c_str();
        texture_info.flip = false;

        LOG_INFO(std::format("Loading {} type {}", texture_path, aiTextureTypeToString(type)));

        Texture& texture = m_texture_cache.get_or_create(texture_path, texture_info);
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
