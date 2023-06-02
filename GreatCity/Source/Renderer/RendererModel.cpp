#include "Renderer/RendererModel.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererVertexBuffer.h"
#include "Renderer/RendererIndexBuffer.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"
#include "Math/Matrix4x4.h"

#include <vector>
#include <unordered_map>

#include <tiny_obj_loader.h>

static void GCRendererModel_HashCombine(std::size_t& Seed)
{
	(void)Seed;
}

template<typename Type, typename... TypeRest>
static void GCRendererModel_HashCombine(std::size_t& Seed, const Type& Value, TypeRest... Rest)
{
	Seed ^= std::hash<Type>{}(Value) + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
	GCRendererModel_HashCombine(Seed, Rest...);
}

namespace std
{
	template<>
	struct hash<GCRendererVertex>
	{
		size_t operator()(const GCRendererVertex& Vertex) const
		{
			size_t Seed = 0;
			GCRendererModel_HashCombine(Seed,
				Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z,
				Vertex.Color.X, Vertex.Color.Y, Vertex.Color.Z, Vertex.Color.W,
				Vertex.TextureCoordinate.X, Vertex.TextureCoordinate.Y
			);

			return Seed;
		}
	};
}

GCRendererModel* GCRendererModel_CreateFromFile(const char* const ModelPath, const char* const MaterialPath)
{
	const char* const ModelPaths[1] = { ModelPath };
	const char* const MaterialPaths[1] = { MaterialPath };

	return GCRendererModel_CreateFromFiles(ModelPaths, MaterialPaths, 1);
}

GCRendererModel* GCRendererModel_CreateFromFiles(const char* const* const ModelPaths, const char* const* const MaterialPaths, const uint32_t ModelCount)
{
	GCRendererModel* Model = (GCRendererModel*)GCMemory_Allocate(sizeof(GCRendererModel));
	Model->VertexBuffer = NULL;
	Model->VertexCount = 0;
	Model->IndexBuffer = NULL;
	Model->IndexCount = 0;

	std::vector<GCRendererVertex> Vertices{};
	std::vector<uint32_t> Indices{};
	
	for (uint32_t Counter = 0; Counter < ModelCount; Counter++)
	{
		tinyobj::ObjReader Reader{};

		tinyobj::ObjReaderConfig ReaderConfiguration{};
		ReaderConfiguration.mtl_search_path = MaterialPaths[Counter];
		
		if (!Reader.ParseFromFile(ModelPaths[Counter], ReaderConfiguration))
		{
			if (!Reader.Warning().empty())
			{
				GC_LOG_WARNING("%s", Reader.Warning().c_str());
			}

			if (!Reader.Error().empty())
			{
				GC_ASSERT_WITH_MESSAGE(false, "Failed to load an OBJ file: %s with error: %s", ModelPaths[Counter], Reader.Error().c_str());
			}
		}

		const tinyobj::attrib_t& Attribute = Reader.GetAttrib();
		const std::vector<tinyobj::shape_t>& Shapes = Reader.GetShapes();
		const std::vector<tinyobj::material_t>& Materials = Reader.GetMaterials();

		std::unordered_map<GCRendererVertex, uint32_t> UniqueVertices{};

		for (const tinyobj::shape_t& Shape : Shapes)
		{
			uint32_t IndexOffset = 0;

			for (uint32_t FaceIndex = 0; FaceIndex < Shape.mesh.num_face_vertices.size(); FaceIndex++)
			{
				const uint32_t Face = Shape.mesh.num_face_vertices[FaceIndex];

				for (uint32_t FaceVertex = 0; FaceVertex < Face; FaceVertex++)
				{
					GCRendererVertex Vertex{};

					const tinyobj::index_t Index = Shape.mesh.indices[IndexOffset + FaceVertex];

					if (Index.vertex_index >= 0)
					{
						Vertex.Position = GCVector3_Create(
							Attribute.vertices[3 * Index.vertex_index + 0],
							Attribute.vertices[3 * Index.vertex_index + 1],
							Attribute.vertices[3 * Index.vertex_index + 2]
						);
					}

					if (Index.texcoord_index >= 0)
					{
						Vertex.TextureCoordinate = GCVector2_Create(
							Attribute.texcoords[2 * Index.texcoord_index + 0],
							Attribute.texcoords[2 * Index.texcoord_index + 1]
						);
					}

					const tinyobj::material_t Material = Materials[Shape.mesh.material_ids[FaceIndex]];
					Vertex.Color = GCVector4_Create(Material.diffuse[0], Material.diffuse[1], Material.diffuse[2], 1.0f);

					if (UniqueVertices.count(Vertex) == 0)
					{
						UniqueVertices[Vertex] = static_cast<uint32_t>(Vertices.size());
						Vertices.emplace_back(Vertex);
					}

					Indices.emplace_back(UniqueVertices[Vertex]);
				}

				IndexOffset += Face;
			}
		}
	}

	const GCRendererDevice* const Device = GCRenderer_GetDevice();
	const GCRendererCommandList* const CommandList = GCRenderer_GetCommandList();

	Model->VertexBuffer = GCRendererVertexBuffer_CreateDynamic(Device, CommandList, Vertices.size() * sizeof(GCRendererVertex));
	GCRendererVertexBuffer_SetVertices(Model->VertexBuffer, Vertices.data(), Vertices.size() * sizeof(GCRendererVertex));

	Model->Vertices = static_cast<GCRendererVertex*>(GCMemory_Allocate(Vertices.size() * sizeof(GCRendererVertex)));
	memcpy(Model->Vertices, Vertices.data(), Vertices.size() * sizeof(GCRendererVertex));

	Model->VertexCount = static_cast<uint32_t>(Vertices.size());

	Model->IndexBuffer = GCRendererIndexBuffer_Create(Device, CommandList, Indices.data(), Indices.size() * sizeof(uint32_t));
	Model->IndexCount = static_cast<uint32_t>(Indices.size());

	return Model;
}

void GCRendererModel_SetTransform(GCRendererModel* const Model, const GCMatrix4x4* const Transform)
{
	for (uint32_t Counter = 0; Counter < Model->VertexCount; Counter++)
	{
		const GCVector3 Position = Model->Vertices[Counter].Position;
		const GCVector4 TransformVector = GCMatrix4x4_MultiplyByVector(Transform, GCVector4_Create(Position.X, Position.Y, Position.Z, 1.0f));

		Model->Vertices[Counter].Position = GCVector3_Create(TransformVector.X, TransformVector.Y, TransformVector.Z);
	}

	GCRendererVertexBuffer_SetVertices(Model->VertexBuffer, Model->Vertices, Model->VertexCount * sizeof(GCRendererVertex));
}

void GCRendererModel_Destroy(GCRendererModel* Model)
{
	GCRendererDevice_WaitIdle(GCRenderer_GetDevice());

	GCRendererIndexBuffer_Destroy(Model->IndexBuffer);
	GCRendererVertexBuffer_Destroy(Model->VertexBuffer);

	GCMemory_Free(Model->Vertices);
	GCMemory_Free(Model);
}
