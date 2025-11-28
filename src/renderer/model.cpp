#include "model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Renderer {

Model::Model(const char* file_path)
    : m_directory(file_path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs);
    m_directory = m_directory.substr(0, m_directory.find_last_of('/'));

    process_node(scene->mRootNode, scene);
}

void Model::draw(ShaderProgram& shader)
{
    for (auto& mesh : m_meshes) {
        mesh.draw(shader);
    }
}

void Model::process_node(aiNode* node, const aiScene* scene)
{
    for (u32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(process_mesh(mesh, scene));
    }

    std::println("node->mNumChildren: {}", node->mNumChildren);
    for (u32 i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}

Mesh Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Mesh::Vertex> vertices;
    std::vector<u32> indices;
    std::vector<TextureRef> textures;

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

        vertices.push_back(vertex);
    }

    for (u32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (u32 j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<TextureRef> diffuseMaps = load_material_textures(material,
            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<TextureRef> specularMaps = load_material_textures(material,
            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh { std::move(vertices), std::move(indices), std::move(textures) };
}

std::vector<TextureRef> Model::load_material_textures(aiMaterial* mat, aiTextureType type, const char* type_name)
{
    std::vector<TextureRef> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        TextureRef texture {};
        std::string texture_path = (m_directory + "/" + str.C_Str());
        bool skip = false;

        for (u32 j = 0; j < m_textures.size(); j++) {
            if (m_textures[j].file_path == texture_path) {
                textures.push_back(TextureRef { .m_tex = &m_textures[j], .m_id = j, .m_type = type_name });
                skip = true;
            }
        }

        if (!skip) {
            TextureInfo texture_info;
            texture_info.from_file = GL_TRUE;
            texture_info.file_path = texture_path.c_str();

            m_textures.emplace_back(Texture { texture_info });

            texture.m_id = static_cast<u32>(m_textures.size()) - 1;
            texture.m_tex = &m_textures.at(texture.m_id);
            texture.m_type = type_name;

            textures.push_back(texture);
        }
    }
    return textures;
}

} // namespace Renderer
