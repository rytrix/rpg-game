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
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs);
    m_directory = m_directory.substr(0, m_directory.find_last_of('/'));

    util_assert(scene->mRootNode != nullptr, "Model::Model: Root node is nullptr");
    process_node(scene->mRootNode, scene);

    m_mesh.setup_mesh();

    initialized = true;
}

Model::~Model()
{
    initialized = false;
}

void Model::draw_untextured(ShaderProgram& shader, glm::mat4 model) const
{
    util_assert(initialized == true, "Model has not been initialized");

    shader.set_mat4("model", model);
    // for (auto& mesh : m_meshes) {
    //     mesh.draw();
    // }
    m_mesh.draw();
}

void Model::draw(ShaderProgram& shader, glm::mat4 model) const
{
    util_assert(initialized == true, "Model has not been initialized");
    shader.set_mat4("model", model);
    // for (auto& mesh : m_meshes) {
    //     mesh.draw(shader);
    // }

    m_mesh.draw(shader);
}

// TODO
// const std::deque<Mesh>* Model::get_meshes()
// {
//     util_assert(initialized == true, "Model has not been initialized");
//     return &m_meshes;
// }

const Mesh* Model::get_mesh()
{
    return &m_mesh;
}

void Model::process_node(aiNode* node, const aiScene* scene)
{
    // std::println("node->mNumMeshes: {}", node->mNumMeshes);
    for (u32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh, scene);
    }

    // std::println("node->mNumChildren: {}", node->mNumChildren);
    for (u32 i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}

void Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    // std::vector<Mesh::Vertex> vertices;
    // std::vector<u32> indices;
    // std::vector<TextureRef> textures;

    GLsizei base_vertex = static_cast<GLsizei>(m_mesh.m_vertices.size());
    GLsizei count = static_cast<GLsizei>(m_mesh.m_indices.size());

    for (u32 i = 0; i < mesh->mNumVertices; i++) {
        Mesh::Vertex vertex {};
        vertex.m_pos.x = mesh->mVertices[i].x;
        vertex.m_pos.y = mesh->mVertices[i].y;
        vertex.m_pos.z = mesh->mVertices[i].z;

        vertex.m_norm.x = mesh->mNormals[i].x;
        vertex.m_norm.y = mesh->mNormals[i].y;
        vertex.m_norm.z = mesh->mNormals[i].z;

        if (mesh->HasTextureCoords(0)) {
            vertex.m_tex.x = mesh->mTextureCoords[0][i].x;
            vertex.m_tex.y = mesh->mTextureCoords[0][i].y;
        }

        m_mesh.m_vertices.push_back(vertex);
        // vertices.push_back(vertex);
    }

    for (u32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (u32 j = 0; j < face.mNumIndices; j++) {
            m_mesh.m_indices.push_back(face.mIndices[j]);
            // indices.push_back(face.mIndices[j]);
        }
    }

    if (scene->HasMaterials()) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<TextureRef> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE);
        // std::println("diffuse size = {}", diffuseMaps.size());
        m_mesh.m_textures.insert(m_mesh.m_textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<TextureRef> specularMaps = load_material_textures(material, aiTextureType_SPECULAR);
        // std::println("specular size = {}", specularMaps.size());
        m_mesh.m_textures.insert(m_mesh.m_textures.end(), specularMaps.begin(), specularMaps.end());
        // textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    count = static_cast<GLsizei>(m_mesh.m_indices.size()) - count;

    m_mesh.m_base_vertices.emplace_back(
        count,
        base_vertex);

    // m_meshes.emplace_back(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<TextureRef> Model::load_material_textures(aiMaterial* mat, aiTextureType type)
{
    std::vector<TextureRef> textures;

    // std::println("{} Material texture count {}", type_name, mat->GetTextureCount(type));
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string texture_path = (m_directory + "/" + str.C_Str());

        TextureInfo texture_info;
        texture_info.from_file = GL_TRUE;
        texture_info.file_path = texture_path.c_str();
        texture_info.flip = false;

        LOG_INFO(std::format("Loading {}", texture_path));

        Texture& texture = m_texture_cache.get_or_create(texture_path, texture_info);
        textures.emplace_back(&texture, type);
    }

    return textures;
}

} // namespace Renderer
