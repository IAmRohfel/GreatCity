#include "Renderer/RendererModel.h"
#include "Renderer/Renderer.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

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

GCRendererModel* GCRendererModel_CreateFromFile(const char* const* const ModelPaths, const char* const* const MaterialPaths, const uint32_t ModelCount)
{
	GCRendererModel* Model = (GCRendererModel*)GCMemory_Allocate(sizeof(GCRendererModel));
	Model->Vertices = NULL;
	Model->VertexCount = 0;
	Model->Indices = NULL;
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

			for (const uint32_t Face : Shape.mesh.num_face_vertices)
			{
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

					if (UniqueVertices.count(Vertex) == 0)
					{
						UniqueVertices[Vertex] = static_cast<uint32_t>(Vertices.size());
						Vertices.emplace_back(Vertex);
					}

					Indices.emplace_back(UniqueVertices[Vertex]);
				}

				IndexOffset += Face;

				const tinyobj::material_t Material = Materials[Shape.mesh.material_ids[Face]];

				for (GCRendererVertex& Vertex : Vertices)
				{
					Vertex.Color = GCVector4_Create(Material.diffuse[0], Material.diffuse[1], Material.diffuse[2], 1.0f);
				}

				//(void)Material;
			}
		}
	}

	Model->Vertices = static_cast<GCRendererVertex*>(GCMemory_Allocate(Vertices.size() * sizeof(GCRendererVertex)));
	memcpy(Model->Vertices, Vertices.data(), Vertices.size() * sizeof(GCRendererVertex));
	Model->VertexCount = static_cast<uint32_t>(Vertices.size());

	Model->Indices = static_cast<uint32_t*>(GCMemory_Allocate(Indices.size() * sizeof(uint32_t)));
	memcpy(Model->Indices, Indices.data(), Indices.size() * sizeof(uint32_t));
	Model->IndexCount = static_cast<uint32_t>(Indices.size());

	return Model;
}

void GCRendererModel_Destroy(GCRendererModel* Model)
{
	GCMemory_Free(Model->Indices);
	GCMemory_Free(Model->Vertices);

	GCMemory_Free(Model);
}
