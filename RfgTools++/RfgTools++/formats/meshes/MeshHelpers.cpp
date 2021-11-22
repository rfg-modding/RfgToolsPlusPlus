#include "MeshHelpers.h"
#include "common/filesystem/Path.h"
#include "types/Vec4.h"
#include <ext/gltf.h>
#include <filesystem>
#include <limits>

bool MeshHelpers::IsExportSupported(VertexFormat format)
{
    switch (format)
    {
        case VertexFormat::Pixlit1Uv:
        case VertexFormat::Pixlit1UvNmap:
        case VertexFormat::Pixlit1UvNmapCa:
        case VertexFormat::Pixlit2UvNmap:
        case VertexFormat::Pixlit3UvNmap:
        case VertexFormat::Pixlit3UvNmapCa:
            return true;
        default:
            return false;
    }
}

bool MeshHelpers::WriteToGltf(const MeshDataBlock& meshInfo, u32 numLods, std::span<u8> indices, std::span<u8> vertices, const string& outPath,
                              const string& diffusePath, const string& specularPath, const string& normalPath)
{
    bool hasDiffuse = diffusePath != "";
    bool hasSpecular = specularPath != "";
    bool hasNormal = normalPath != "";

    //Ensure format is supported
    if (!IsExportSupported(meshInfo.VertFormat))
        return false;

    //Convert vertices to gltf compatible format
    std::optional<std::span<u8>> maybeGltfVertices = MeshHelpers::ConvertVerticesToGltfFormat(meshInfo, vertices);
    defer(if (maybeGltfVertices.has_value()) delete[] maybeGltfVertices.value().data());
    if (!maybeGltfVertices)
        return false;
    std::span<u8> verticesGltfBytes = maybeGltfVertices.value();

    //Determine index type
    int indexType = 0;
    if (meshInfo.IndexSize == 2)
        indexType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
    else if (meshInfo.IndexSize == 4)
        indexType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    else
        throw std::runtime_error("Unsupported index size " + std::to_string(meshInfo.IndexSize));

    //Determine primitive topology
    int primitiveTopology = 0;
    if (meshInfo.PrimitiveType == PrimitiveTopology::TriangleStrip)
        primitiveTopology = TINYGLTF_MODE_TRIANGLE_STRIP;
    else if (meshInfo.PrimitiveType == PrimitiveTopology::TriangleList)
        primitiveTopology = TINYGLTF_MODE_TRIANGLES;
    else
        throw std::runtime_error("Unsupported primitive topology " + std::to_string((i32)meshInfo.PrimitiveType));

    //Find min and max vertex positions & calculate bounding box size
    Vec3 posMin;
    Vec3 posMax;
    std::span<GltfVertex> verticesGltfTyped = ToTypedSpan<GltfVertex>(verticesGltfBytes);
    for (const GltfVertex& vert : verticesGltfTyped)
    {
        //Min
        if (vert.Position.x < posMin.x) posMin.x = vert.Position.x;
        if (vert.Position.y < posMin.y) posMin.y = vert.Position.y;
        if (vert.Position.z < posMin.z) posMin.z = vert.Position.z;
        //Max
        if (vert.Position.x > posMax.x) posMax.x = vert.Position.x;
        if (vert.Position.y > posMax.y) posMax.y = vert.Position.y;
        if (vert.Position.z > posMax.z) posMax.z = vert.Position.z;
    }
    const Vec3 size = posMax - posMin;

    //Gltf file data
    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Asset asset;
    tinygltf::Buffer indexBuffer;
    tinygltf::Buffer vertexBuffer;
    tinygltf::BufferView vertexBufferView;
    tinygltf::Accessor positionAccessor;
    tinygltf::Accessor normalAccessor;
    tinygltf::Accessor tangentAccessor;
    tinygltf::Accessor texcoord0Accessor;
    tinygltf::Node rootNode;

    //Root node to store all meshes in. Makes things nicely organized on import into blender.
    rootNode.name = Path::GetFileNameNoExtension(outPath);

    //Index buffer
    const int indexBufferIndex = static_cast<int>(model.buffers.size());
    indexBuffer.data = ToByteVector(indices);//ToByteVector(blockIndices);
    indexBuffer.name = "IndexBuffer";
    model.buffers.push_back(indexBuffer);

    //Vertex buffer
    const int vertexBufferIndex = static_cast<int>(model.buffers.size());
    vertexBuffer.data = ToByteVector(verticesGltfBytes);
    vertexBuffer.name = "VertexBuffer";
    model.buffers.push_back(vertexBuffer);

    //Vertex buffer view
    const int vertexBufferViewIndex = static_cast<int>(model.bufferViews.size());
    vertexBufferView.buffer = vertexBufferIndex;
    vertexBufferView.byteOffset = 0;
    vertexBufferView.byteLength = vertexBuffer.data.size();
    vertexBufferView.byteStride = sizeof(GltfVertex);
    vertexBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    model.bufferViews.push_back(vertexBufferView);

    //Vertex accessors. Used for all submeshes. Index accessors vary by render block.
    const int positionAccessorIndex = static_cast<int>(model.accessors.size());
    positionAccessor.bufferView = vertexBufferViewIndex;
    positionAccessor.byteOffset = offsetof(GltfVertex, Position);
    positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    positionAccessor.count = meshInfo.NumVertices;
    positionAccessor.type = TINYGLTF_TYPE_VEC3;
    positionAccessor.name = "PositionAccessor";
    model.accessors.push_back(positionAccessor);

    const int normalAccessorIndex = static_cast<int>(model.accessors.size());
    normalAccessor.bufferView = vertexBufferViewIndex;
    normalAccessor.byteOffset = offsetof(GltfVertex, Normal);
    normalAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    normalAccessor.count = meshInfo.NumVertices;
    normalAccessor.type = TINYGLTF_TYPE_VEC3;
    normalAccessor.name = "NormalAccessor";
    model.accessors.push_back(normalAccessor);

    const int tangentAccessorIndex = static_cast<int>(model.accessors.size());
    tangentAccessor.bufferView = vertexBufferViewIndex;
    tangentAccessor.byteOffset = offsetof(GltfVertex, Tangent);
    tangentAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    tangentAccessor.count = meshInfo.NumVertices;
    tangentAccessor.type = TINYGLTF_TYPE_VEC4;
    tangentAccessor.name = "TangentAccessor";
    model.accessors.push_back(tangentAccessor);

    const int texcoord0AccessorIndex = static_cast<int>(model.accessors.size());
    texcoord0Accessor.bufferView = vertexBufferViewIndex;
    texcoord0Accessor.byteOffset = offsetof(GltfVertex, Texcoord0);
    texcoord0Accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    texcoord0Accessor.count = meshInfo.NumVertices;
    texcoord0Accessor.type = TINYGLTF_TYPE_VEC2;
    texcoord0Accessor.name = "Texcoord0Accessor";
    model.accessors.push_back(texcoord0Accessor);

    //Material used by all meshes
    const int materialIndex = static_cast<int>(model.materials.size());
    tinygltf::Material& material = model.materials.emplace_back();
    material.pbrMetallicRoughness.baseColorFactor = { 1.0f, 0.9f, 0.9f, 1.0f };
    material.doubleSided = true;

    //Creates a gltf texture, sampler, and image for a texture file
    auto addImage = [&](const std::string& texturePath) -> tinygltf::Image&
    {
        tinygltf::Texture& texture = model.textures.emplace_back();
        texture.source = static_cast<int>(model.images.size());
        texture.sampler = static_cast<int>(model.samplers.size());

        tinygltf::Sampler& sampler = model.samplers.emplace_back();
        sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
        sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
        sampler.wrapS = TINYGLTF_TEXTURE_WRAP_REPEAT;
        sampler.wrapT = TINYGLTF_TEXTURE_WRAP_REPEAT;

        tinygltf::Image& image = model.images.emplace_back();
        image.uri = texturePath;
        return image;
    };

    //Add textures to material
    if (hasDiffuse)
    {
        addImage(diffusePath);
        material.pbrMetallicRoughness.baseColorTexture.index = static_cast<int>(model.textures.size()) - 1;
    }
    if (hasSpecular)
    {
        //Note: There isn't really a direct translation of specular textures to the PBR model that gltf uses.
        //This is only here so blender auto loads the texture. Most meshes will require some tweaking to look good in PBR renderers.
        addImage(specularPath);
        material.pbrMetallicRoughness.metallicRoughnessTexture.index = static_cast<int>(model.textures.size()) - 1;
    }
    if (hasNormal)
    {
        addImage(normalPath);
        material.normalTexture.index = static_cast<int>(model.textures.size()) - 1;
    }

    //Create gltf meshes
    const u32 submeshesPerLodLevel = static_cast<int>(meshInfo.Submeshes.size()) / numLods;
    for (u32 i = 0; i < meshInfo.Submeshes.size(); i++)
    {
        const SubmeshData& submesh = meshInfo.Submeshes[i];
        tinygltf::Node submeshNode; //Used if there are multiple submeshes
        if (numLods > 1)
            submeshNode.name = "LOD" + std::to_string(i / submeshesPerLodLevel);
        else
            submeshNode.name = "Submesh " + std::to_string(i);

        //Move each lod level further along the x axis so they don't overlap
        if (numLods > 1)
        {
            u32 lodLevel = i / submeshesPerLodLevel;
            submeshNode.translation.push_back(2.0 * size.x * lodLevel);
            submeshNode.translation.push_back(0.0);
            submeshNode.translation.push_back(0.0);
        }

        //One gltf mesh per render block
        u32 firstBlock = submesh.RenderBlocksOffset;
        for (u32 j = 0; j < submesh.NumRenderBlocks; j++)
        {
            //Per mesh data
            const RenderBlock& block = meshInfo.RenderBlocks[firstBlock + j];
            std::span<u8> blockIndices = { indices.data() + (block.StartIndex * meshInfo.IndexSize), block.NumIndices * meshInfo.IndexSize };
            tinygltf::Mesh mesh;
            tinygltf::Primitive primitive;
            tinygltf::BufferView indexBufferView;
            tinygltf::Accessor indexAccessor;

            //Index buffer view
            const int indexBufferViewIndex = static_cast<int>(model.bufferViews.size());
            indexBufferView.buffer = indexBufferIndex;
            indexBufferView.byteOffset = block.StartIndex * meshInfo.IndexSize;
            indexBufferView.byteLength = block.NumIndices * meshInfo.IndexSize;
            indexBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
            model.bufferViews.push_back(indexBufferView);

            //Index accessor
            primitive.indices = static_cast<int>(model.accessors.size());
            indexAccessor.bufferView = indexBufferViewIndex;
            indexAccessor.byteOffset = 0;
            indexAccessor.componentType = indexType;
            indexAccessor.count = block.NumIndices;
            indexAccessor.type = TINYGLTF_TYPE_SCALAR;
            indexAccessor.name = "IndexAccessor (submesh " + std::to_string(i) + ")(block " + std::to_string(j) + ")";
            model.accessors.push_back(indexAccessor);

            //Vertex attributes
            primitive.attributes["POSITION"] = positionAccessorIndex;
            primitive.attributes["NORMAL"] = normalAccessorIndex;
            primitive.attributes["TANGENT"] = tangentAccessorIndex;
            primitive.attributes["TEXCOORD_0"] = texcoord0AccessorIndex;

            //Gltf primitive
            primitive.material = materialIndex;
            primitive.mode = primitiveTopology;
            mesh.primitives.push_back(primitive);

            //Add submesh to model
            tinygltf::Node node;
            node.name = "Block" + std::to_string(j);
            node.mesh = static_cast<int>(model.meshes.size());
            const int nodeIndex = static_cast<int>(model.nodes.size());
            model.meshes.push_back(mesh);
            model.nodes.push_back(node);

            //Organize each submesh into their own subnode if > 1 submesh
            if (meshInfo.Submeshes.size() > 1)
                submeshNode.children.push_back(nodeIndex);
            else //Otherwise put in the models root node
                rootNode.children.push_back(nodeIndex);
        }

        //Organize each submesh into their own subnode if > 1 submesh
        if (meshInfo.Submeshes.size() > 1)
        {
            rootNode.children.push_back(static_cast<int>(model.nodes.size()));
            model.nodes.push_back(submeshNode);
        }
    }

    //Add scene and root node to model
    model.nodes.push_back(rootNode);
    scene.nodes.push_back(static_cast<int>(scene.nodes.size()));
    model.scenes.push_back(scene);

    //Asset metadata
    asset.version = "2.0";
    asset.generator = "tinygltf";
    model.asset = asset;

    //Output gltf file
    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&model, outPath,
        false,  //Embed images
        true,   //Embed buffers
        true,   //Pretty print
        false); //Write binary

    return true;
}

//Used by MeshHelpers::ConvertVerticesToGltfFormat()
struct Vec4_UByte
{
    u8 x, y, z, w;
};
struct Vec2_I16
{
    i16 x, y;
};

std::optional<std::span<u8>> MeshHelpers::ConvertVerticesToGltfFormat(const MeshDataBlock& meshInfo, std::span<u8> rfgVertices)
{
    //Create buffer for gltf vertices
    std::span<GltfVertex> verticesGltf = { new GltfVertex[meshInfo.NumVertices], meshInfo.NumVertices };

    //Get rfg vertices data layout
    std::optional<VertexFormatLayout> maybeVertexLayout = GetVertexFormatLayout(meshInfo.VertFormat);
    if (!maybeVertexLayout)
    {
        delete[] verticesGltf.data();
        return {};
    }
    const VertexFormatLayout formatLayout = maybeVertexLayout.value();

    //Convert each rfg vertex to a gltf vertex
    for (u32 i = 0; i < meshInfo.NumVertices; i++)
    {
        //Vertices
        u8* rfgVert = rfgVertices.data() + (i * meshInfo.VertexStride0);
        GltfVertex& gltfVert = verticesGltf[i];

        //Convert components which are useable by gltf
        for (const VertexComponentLayout& component : formatLayout.Components)
        {
            if (component.Name == "POSITION")
            {
                if (component.Format == F32_3)
                {
                    const Vec3& position = *(Vec3*)(rfgVert + component.Offset);
                    gltfVert.Position = position;
                }
            }
            else if (component.Name == "NORMAL")
            {
                if (component.Format == U8_4_UNORM)
                {
                    const Vec4_UByte& normal = *(Vec4_UByte*)(rfgVert + component.Offset);
                    f32 u8Max = (f32)std::numeric_limits<u8>::max();
                    gltfVert.Normal = { (f32)normal.x / u8Max, (f32)normal.y / u8Max, (f32)normal.z / u8Max };

                    //Adjust from [0, 1] to [-1, 1]
                    gltfVert.Normal -= 0.5f;
                    gltfVert.Normal *= 2.0f;
                }
            }
            else if (component.Name == "TANGENT")
            {
                if (component.Format == U8_4_UNORM)
                {
                    const Vec4_UByte& tangent = *(Vec4_UByte*)(rfgVert + component.Offset);
                    f32 u8Max = (f32)std::numeric_limits<u8>::max();
                    gltfVert.Tangent = { (f32)tangent.x / u8Max, (f32)tangent.y / u8Max, (f32)tangent.z / u8Max, (f32)tangent.w / u8Max };

                    //Adjust from [0, 1] to [-1, 1]
                    gltfVert.Normal -= 0.5f;
                    gltfVert.Normal *= 2.0f;
                }
            }
            else if (component.Name == "TEXCOORD_0")
            {
                if (component.Format == I16_2)
                {
                    const Vec2_I16& uv = *(Vec2_I16*)(rfgVert + component.Offset);
                    f32 texcoordDivisor = 1024.0f; //This is how RFG shaders normalize UVs. Don't know why they don't use i16_max.
                    gltfVert.Texcoord0 = { (f32)uv.x / texcoordDivisor, (f32)uv.y / texcoordDivisor };
                }
            }
            //Todo: Support other components. Some RFG meshes have 2-3 UVs and animation blend data.
            //Todo: Support other component formats. E.g. Some terrain meshes don't use Vec3 for position.
        }
    }

    return std::span<u8>{ (u8*)verticesGltf.data(), verticesGltf.size() * sizeof(GltfVertex) };
}