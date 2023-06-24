/*
    Copyright (C) 2023  Rohfel Adyaraka Christianugrah Puspoasmoro

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Renderer/Renderer.h"
#include "ApplicationCore/Application.h"
#include "ApplicationCore/GenericPlatform/Window.h"
#include "Core/Memory/Allocator.h"
#include "ImGui/ImGuiManager.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Renderer/RendererCommandList.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererEnums.h"
#include "Renderer/RendererFramebuffer.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Renderer/RendererIndexBuffer.h"
#include "Renderer/RendererMesh.h"
#include "Renderer/RendererShader.h"
#include "Renderer/RendererSwapChain.h"
#include "Renderer/RendererTexture2D.h"
#include "Renderer/RendererUniformBuffer.h"
#include "Renderer/RendererVertexBuffer.h"
#include "World/Camera/WorldCamera.h"
#include "World/Components.h"
#include "World/Entity.h"

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct GCRendererDrawData
{
    const GCRendererVertexBuffer* VertexBuffer;
    uint32_t VertexCount;

    const GCRendererIndexBuffer* IndexBuffer;
    uint32_t IndexCount;
} GCRendererDrawData;

typedef struct GCRenderer
{
    GCRendererDevice* Device;
    GCRendererSwapChain* SwapChain;
    GCRendererCommandList* CommandList;
    GCRendererUniformBuffer* UniformBuffer;
    GCRendererTexture2D* WhiteTexture2D;
    GCRendererShader* BasicShader;
    GCRendererGraphicsPipeline* GraphicsPipeline;
    GCRendererFramebuffer* Framebuffer;

    GCRendererGraphicsPipelineDescriptor GraphicsPipelineDescriptors[2];
    uint32_t GraphicsPipelineDescriptorCount;

    uint32_t MaximumQuads;
    uint32_t MaximumQuadVertices, MaximumQuadIndices;
    uint32_t QuadIndexCount;
    GCVector3 QuadVertexPositions[4];
    GCVector3 QuadVertexNormals[4];
    GCVector2 QuadTextureCoordinates[4];
    GCRendererVertexBuffer* QuadVertexBuffer;
    GCRendererIndexBuffer* QuadIndexBuffer;
    GCRendererVertex *QuadVertexBufferBase, *QuadVertexBufferPointer;

    uint32_t MaximumTextureSlots;
    uint32_t Texture2DSlotIndex;
    const GCRendererTexture2D** Texture2DSlots;

    uint32_t MaximumDrawDataCount;
    GCRendererDrawData* DrawData;
    uint32_t DrawDataCount;
} GCRenderer;

typedef struct GCRendererUniformBufferData
{
    alignas(16) GCMatrix4x4 ViewProjectionMatrix;
} GCRendererUniformBufferData;

static void GCRenderer_ResizeSwapChain(void);
static void GCRenderer_BeginBatch(void);
static void GCRenderer_EndBatch(void);
static void GCRenderer_NextBatch(void);

static GCRenderer* Renderer = NULL;

void GCRenderer_Initialize(void)
{
    Renderer = (GCRenderer*)GCMemory_AllocateZero(sizeof(GCRenderer));
    Renderer->Device = GCRendererDevice_Create();

    GCRendererSwapChainDescription SwapChainDescription = {0};
    SwapChainDescription.Device = Renderer->Device;
    Renderer->SwapChain = GCRendererSwapChain_Create(&SwapChainDescription);

    GCRendererCommandListDescription CommandListDescription = {0};
    CommandListDescription.Device = Renderer->Device;
    CommandListDescription.SwapChain = Renderer->SwapChain;
    Renderer->CommandList = GCRendererCommandList_Create(&CommandListDescription);

    GCRendererUniformBufferDescription UniformBufferDescription = {0};
    UniformBufferDescription.Device = Renderer->Device;
    UniformBufferDescription.CommandList = Renderer->CommandList;
    UniformBufferDescription.DataSize = sizeof(GCRendererUniformBufferData);
    Renderer->UniformBuffer = GCRendererUniformBuffer_Create(&UniformBufferDescription);

    GCRendererTexture2DDescription WhiteTexture2DDescription = {0};
    WhiteTexture2DDescription.Device = Renderer->Device;
    WhiteTexture2DDescription.CommandList = Renderer->CommandList;
    WhiteTexture2DDescription.Width = 1;
    WhiteTexture2DDescription.Height = 1;
    WhiteTexture2DDescription.Format = GCRendererFormat_SRGB;
    Renderer->WhiteTexture2D = GCRendererTexture2D_CreateDynamic(&WhiteTexture2DDescription);

    uint32_t TextureData = 0xffffffff;
    GCRendererTexture2D_SetTextureDataDynamic(Renderer->WhiteTexture2D, 1, 1, &TextureData);
    GCRendererTexture2D_FinalizeDynamic(Renderer->WhiteTexture2D);

    GCRendererShaderDescription ShaderDescription = {0};
    ShaderDescription.Device = Renderer->Device;
    ShaderDescription.VertexShaderPath = "Assets/Shaders/Basic/Basic.vertex.glsl";
    ShaderDescription.FragmentShaderPath = "Assets/Shaders/Basic/Basic.fragment.glsl";
    Renderer->BasicShader = GCRendererShader_Create(&ShaderDescription);

    GCRendererCommandList_SetSwapChainResizeCallback(Renderer->CommandList, GCRenderer_ResizeSwapChain);

    Renderer->MaximumQuads = 100;
    Renderer->MaximumQuadVertices = Renderer->MaximumQuads * 4;
    Renderer->MaximumQuadIndices = Renderer->MaximumQuads * 6;

    Renderer->QuadVertexPositions[0] = GCVector3_Create(-0.5f, -0.5f, 0.0f);
    Renderer->QuadVertexPositions[1] = GCVector3_Create(0.5f, -0.5f, 0.0f);
    Renderer->QuadVertexPositions[2] = GCVector3_Create(0.5f, 0.5f, 0.0f);
    Renderer->QuadVertexPositions[3] = GCVector3_Create(-0.5f, 0.5f, 0.0f);

    Renderer->QuadVertexNormals[0] = GCVector3_Cross(
        GCVector3_Subtract(Renderer->QuadVertexPositions[1], Renderer->QuadVertexPositions[0]),
        GCVector3_Subtract(Renderer->QuadVertexPositions[3], Renderer->QuadVertexPositions[0])
    );
    Renderer->QuadVertexNormals[1] = GCVector3_Cross(
        GCVector3_Subtract(Renderer->QuadVertexPositions[0], Renderer->QuadVertexPositions[1]),
        GCVector3_Subtract(Renderer->QuadVertexPositions[2], Renderer->QuadVertexPositions[1])
    );
    Renderer->QuadVertexNormals[2] = GCVector3_Cross(
        GCVector3_Subtract(Renderer->QuadVertexPositions[3], Renderer->QuadVertexPositions[2]),
        GCVector3_Subtract(Renderer->QuadVertexPositions[1], Renderer->QuadVertexPositions[2])
    );
    Renderer->QuadVertexNormals[3] = GCVector3_Cross(
        GCVector3_Subtract(Renderer->QuadVertexPositions[2], Renderer->QuadVertexPositions[3]),
        GCVector3_Subtract(Renderer->QuadVertexPositions[0], Renderer->QuadVertexPositions[3])
    );

    Renderer->QuadTextureCoordinates[0] = GCVector2_Create(1.0f, 0.0f);
    Renderer->QuadTextureCoordinates[1] = GCVector2_Create(0.0f, 0.0f);
    Renderer->QuadTextureCoordinates[2] = GCVector2_Create(0.0f, 1.0f);
    Renderer->QuadTextureCoordinates[3] = GCVector2_Create(1.0f, 1.0f);

    GCRendererVertexBufferDescription QuadVertexBufferDescription = {0};
    QuadVertexBufferDescription.Device = Renderer->Device;
    QuadVertexBufferDescription.CommandList = Renderer->CommandList;
    QuadVertexBufferDescription.Vertices = NULL;
    QuadVertexBufferDescription.VertexCount = Renderer->MaximumQuadVertices;
    QuadVertexBufferDescription.VertexSize = Renderer->MaximumQuadVertices * sizeof(GCRendererVertex);
    Renderer->QuadVertexBuffer = GCRendererVertexBuffer_CreateDynamic(&QuadVertexBufferDescription);

    uint32_t* QuadIndices = (uint32_t*)GCMemory_AllocateZero(Renderer->MaximumQuadIndices * sizeof(uint32_t));

    uint32_t QuadIndexOffset = 0;
    for (uint32_t Counter = 0; Counter < Renderer->MaximumQuadIndices; Counter += 6)
    {
        QuadIndices[Counter + 0] = QuadIndexOffset + 0;
        QuadIndices[Counter + 1] = QuadIndexOffset + 1;
        QuadIndices[Counter + 2] = QuadIndexOffset + 2;

        QuadIndices[Counter + 3] = QuadIndexOffset + 2;
        QuadIndices[Counter + 4] = QuadIndexOffset + 3;
        QuadIndices[Counter + 5] = QuadIndexOffset + 0;

        QuadIndexOffset += 4;
    }

    GCRendererIndexBufferDescription QuadIndexBufferDescription = {0};
    QuadIndexBufferDescription.Device = Renderer->Device;
    QuadIndexBufferDescription.CommandList = Renderer->CommandList;
    QuadIndexBufferDescription.Indices = QuadIndices;
    QuadIndexBufferDescription.IndexCount = Renderer->MaximumQuadIndices;
    QuadIndexBufferDescription.IndexSize = Renderer->MaximumQuadIndices * sizeof(uint32_t);
    Renderer->QuadIndexBuffer = GCRendererIndexBuffer_Create(&QuadIndexBufferDescription);

    GCMemory_Free(QuadIndices);

    Renderer->QuadVertexBufferBase =
        (GCRendererVertex*)GCMemory_AllocateZero(Renderer->MaximumQuadVertices * sizeof(GCRendererVertex));

    Renderer->MaximumTextureSlots = 32;
    Renderer->Texture2DSlots =
        (GCRendererTexture2D**)GCMemory_AllocateZero(Renderer->MaximumTextureSlots * sizeof(GCRendererTexture2D*));

    for (uint32_t Counter = 0; Counter < Renderer->MaximumTextureSlots; Counter++)
    {
        Renderer->Texture2DSlots[Counter] = Renderer->WhiteTexture2D;
    }

    GCRendererGraphicsPipelineAttachment GraphicsPipelineAttachments[3] = {0};
    GraphicsPipelineAttachments[0].Type = GCRendererAttachmentType_Color;
    GraphicsPipelineAttachments[0].Format = GCRendererFormat_SRGB;
    GraphicsPipelineAttachments[0].SampleCount = GCRendererSampleCount_2;
    GraphicsPipelineAttachments[1].Type = GCRendererAttachmentType_Color;
    GraphicsPipelineAttachments[1].Format = GCRendererFormat_UnsignedInteger;
    GraphicsPipelineAttachments[1].SampleCount = GCRendererSampleCount_2;
    GraphicsPipelineAttachments[2].Type = GCRendererAttachmentType_DepthStencil;
    GraphicsPipelineAttachments[2].Format = GCRendererFormat_D32;
    GraphicsPipelineAttachments[2].SampleCount = GCRendererSampleCount_2;

    GCRendererGraphicsPipelineVertexInputBinding GraphicsPipelineVertexInputBindings[1] = {0};
    GraphicsPipelineVertexInputBindings[0].Binding = 0;
    GraphicsPipelineVertexInputBindings[0].Stride = sizeof(GCRendererVertex);

    GCRendererGraphicsPipelineVertexInputAttribute GraphicsPipelineVertexInputAttributes[6] = {0};
    GraphicsPipelineVertexInputAttributes[0].Location = 0;
    GraphicsPipelineVertexInputAttributes[0].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3;
    GraphicsPipelineVertexInputAttributes[0].Offset = offsetof(GCRendererVertex, Position);

    GraphicsPipelineVertexInputAttributes[1].Location = 1;
    GraphicsPipelineVertexInputAttributes[1].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3;
    GraphicsPipelineVertexInputAttributes[1].Offset = offsetof(GCRendererVertex, Normal);

    GraphicsPipelineVertexInputAttributes[2].Location = 2;
    GraphicsPipelineVertexInputAttributes[2].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector4;
    GraphicsPipelineVertexInputAttributes[2].Offset = offsetof(GCRendererVertex, Color);

    GraphicsPipelineVertexInputAttributes[3].Location = 3;
    GraphicsPipelineVertexInputAttributes[3].Format = GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector2;
    GraphicsPipelineVertexInputAttributes[3].Offset = offsetof(GCRendererVertex, TextureCoordinate);

    GraphicsPipelineVertexInputAttributes[4].Location = 4;
    GraphicsPipelineVertexInputAttributes[4].Format =
        GCRendererGraphicsPipelineVertexInputAttributeFormat_UnsignedInteger;
    GraphicsPipelineVertexInputAttributes[4].Offset = offsetof(GCRendererVertex, TextureIndex);

    GraphicsPipelineVertexInputAttributes[5].Location = 5;
    GraphicsPipelineVertexInputAttributes[5].Format =
        GCRendererGraphicsPipelineVertexInputAttributeFormat_UnsignedInteger;
    GraphicsPipelineVertexInputAttributes[5].Offset = offsetof(GCRendererVertex, EntityID);

    GCRendererGraphicsPipelineVertexInput GraphicsPipelineVertexInput = {0};
    GraphicsPipelineVertexInput.Bindings = GraphicsPipelineVertexInputBindings;
    GraphicsPipelineVertexInput.BindingCount =
        sizeof(GraphicsPipelineVertexInputBindings) / sizeof(GCRendererGraphicsPipelineVertexInputBinding);
    GraphicsPipelineVertexInput.Attributes = GraphicsPipelineVertexInputAttributes;
    GraphicsPipelineVertexInput.AttributeCount =
        sizeof(GraphicsPipelineVertexInputAttributes) / sizeof(GCRendererGraphicsPipelineVertexInputAttribute);

    Renderer->GraphicsPipelineDescriptors[0].Type = GCRendererGraphicsPipelineDescriptorType_UniformBuffer;
    Renderer->GraphicsPipelineDescriptors[0].DescriptorCount = 1;
    Renderer->GraphicsPipelineDescriptors[0].UniformBuffer = Renderer->UniformBuffer;
    Renderer->GraphicsPipelineDescriptors[1].Type = GCRendererGraphicsPipelineDescriptorType_TextureArray;
    Renderer->GraphicsPipelineDescriptors[1].DescriptorCount = Renderer->MaximumTextureSlots;
    Renderer->GraphicsPipelineDescriptors[1].Texture2DArray = Renderer->Texture2DSlots;
    Renderer->GraphicsPipelineDescriptorCount = 2;

    GCRendererGraphicsPipelineDescription GraphicsPipelineDescription = {0};
    GraphicsPipelineDescription.Device = Renderer->Device;
    GraphicsPipelineDescription.SwapChain = Renderer->SwapChain;
    GraphicsPipelineDescription.CommandList = Renderer->CommandList;
    GraphicsPipelineDescription.Attachments = GraphicsPipelineAttachments;
    GraphicsPipelineDescription.AttachmentCount = 3;
    GraphicsPipelineDescription.SampleCount = GCRendererSampleCount_2;
    GraphicsPipelineDescription.VertexInput = GraphicsPipelineVertexInput;
    GraphicsPipelineDescription.Descriptors = Renderer->GraphicsPipelineDescriptors;
    GraphicsPipelineDescription.DescriptorCount = Renderer->GraphicsPipelineDescriptorCount;
    GraphicsPipelineDescription.Shader = Renderer->BasicShader;
    Renderer->GraphicsPipeline = GCRendererGraphicsPipeline_Create(&GraphicsPipelineDescription);

    GCRendererFramebufferAttachment FramebufferAttachments[3] = {0};
    FramebufferAttachments[0].Type = GCRendererAttachmentType_Color;
    FramebufferAttachments[0].Flags = GCRendererFramebufferAttachmentFlags_Sampled;
    FramebufferAttachments[0].Format = GCRendererFormat_SRGB;
    FramebufferAttachments[0].SampleCount = GCRendererSampleCount_2;

    FramebufferAttachments[1].Type = GCRendererAttachmentType_Color;
    FramebufferAttachments[1].Flags = GCRendererFramebufferAttachmentFlags_None;
    FramebufferAttachments[1].Format = GCRendererFormat_UnsignedInteger;
    FramebufferAttachments[1].SampleCount = GCRendererSampleCount_2;

    FramebufferAttachments[2].Type = GCRendererAttachmentType_DepthStencil;
    FramebufferAttachments[2].Flags = GCRendererFramebufferAttachmentFlags_None;
    FramebufferAttachments[2].Format = GCRendererFormat_D32;
    FramebufferAttachments[2].SampleCount = GCRendererSampleCount_2;

    uint32_t WindowWidth = 0, WindowHeight = 0;
    GCWindow_GetWindowSize(GCApplication_GetWindow(), &WindowWidth, &WindowHeight);

    GCRendererFramebufferDescription FramebufferDescription = {0};
    FramebufferDescription.Device = Renderer->Device;
    FramebufferDescription.SwapChain = Renderer->SwapChain;
    FramebufferDescription.GraphicsPipeline = Renderer->GraphicsPipeline;
    FramebufferDescription.Width = WindowWidth;
    FramebufferDescription.Height = WindowHeight;
    FramebufferDescription.Attachments = FramebufferAttachments;
    FramebufferDescription.AttachmentCount = 3;
    Renderer->Framebuffer = GCRendererFramebuffer_Create(&FramebufferDescription);

    Renderer->MaximumDrawDataCount = 100;
    Renderer->DrawData =
        (GCRendererDrawData*)GCMemory_AllocateZero(Renderer->MaximumDrawDataCount * sizeof(GCRendererDrawData));
    Renderer->DrawDataCount = 0;
}

void GCRenderer_BeginWorld(const GCWorldCamera* const WorldCamera)
{
    Renderer->DrawDataCount = 0;

    GCRenderer_BeginBatch();
    GCRendererCommandList_BeginRecord(Renderer->CommandList);

    const float ClearColorTexture[4] = {0.729f, 0.901f, 0.992f, 1.0f};
    GCRendererCommandList_BeginAttachmentRenderPass(
        Renderer->CommandList, Renderer->GraphicsPipeline, Renderer->Framebuffer, ClearColorTexture
    );

    GCRendererCommandList_BindGraphicsPipeline(Renderer->CommandList, Renderer->GraphicsPipeline);
    GCRendererCommandList_SetViewport(Renderer->CommandList, Renderer->Framebuffer);

    GCRendererUniformBufferData UniformBufferData = {0};
    UniformBufferData.ViewProjectionMatrix = GCWorldCamera_GetViewProjectionMatrix(WorldCamera);

    GCRendererUniformBuffer_UpdateUniformBuffer(
        Renderer->UniformBuffer, &UniformBufferData, sizeof(GCRendererUniformBufferData)
    );
}

void GCRenderer_DrawQuad(const GCVector3 Position, const GCVector2 Size, const GCVector4 Color)
{
    const GCMatrix4x4 Translation = GCMatrix4x4_CreateTranslation(Position);
    const GCMatrix4x4 Scale = GCMatrix4x4_CreateScale(GCVector3_Create(Size.X, Size.Y, 1.0f));
    const GCMatrix4x4 Transform = GCMatrix4x4_Multiply(&Translation, &Scale);

    GCRenderer_DrawQuadTransform(&Transform, Color);
}

void GCRenderer_DrawQuadTexture2D(
    const GCVector3 Position, const GCVector2 Size, const GCRendererTexture2D* const Texture2D,
    const GCVector4 TintColor
)
{
    const GCMatrix4x4 Translation = GCMatrix4x4_CreateTranslation(Position);
    const GCMatrix4x4 Scale = GCMatrix4x4_CreateScale(GCVector3_Create(Size.X, Size.Y, 1.0f));
    const GCMatrix4x4 Transform = GCMatrix4x4_Multiply(&Translation, &Scale);

    GCRenderer_DrawQuadTexture2DTransform(&Transform, Texture2D, TintColor);
}

void GCRenderer_DrawQuadTransform(const GCMatrix4x4* const Transform, const GCVector4 Color)
{
    if (Renderer->QuadIndexCount >= Renderer->MaximumQuadIndices)
    {
        GCRenderer_NextBatch();
    }

    for (uint32_t Counter = 0; Counter < 4; Counter++)
    {
        const GCVector4 TransformVector = GCMatrix4x4_MultiplyByVector(
            Transform, GCVector4_Create(
                           Renderer->QuadVertexPositions[Counter].X, Renderer->QuadVertexPositions[Counter].Y,
                           Renderer->QuadVertexPositions[Counter].Z, 1.0f
                       )
        );

        Renderer->QuadVertexBufferPointer->Position =
            GCVector3_Create(TransformVector.X, TransformVector.Y, TransformVector.Z);
        Renderer->QuadVertexBufferPointer->Normal = Renderer->QuadVertexNormals[Counter];
        Renderer->QuadVertexBufferPointer->Color = Color;
        Renderer->QuadVertexBufferPointer->TextureCoordinate = Renderer->QuadTextureCoordinates[Counter];
        Renderer->QuadVertexBufferPointer->TextureIndex = 0;
        Renderer->QuadVertexBufferPointer->EntityID = 0;
        Renderer->QuadVertexBufferPointer++;
    }

    Renderer->QuadIndexCount += 6;
}

void GCRenderer_DrawQuadTexture2DTransform(
    const GCMatrix4x4* const Transform, const GCRendererTexture2D* const Texture2D, const GCVector4 TintColor
)
{
    if (Renderer->QuadIndexCount >= Renderer->MaximumQuadIndices)
    {
        GCRenderer_NextBatch();
    }

    uint32_t TextureIndex = 0;

    for (uint32_t Counter = 1; Counter < Renderer->Texture2DSlotIndex; Counter++)
    {
        if (Renderer->Texture2DSlots[Counter] == Texture2D)
        {
            TextureIndex = Counter;

            break;
        }
    }

    if (!TextureIndex)
    {
        if (Renderer->Texture2DSlotIndex >= Renderer->MaximumTextureSlots)
        {
            GCRenderer_NextBatch();
        }

        TextureIndex = Renderer->Texture2DSlotIndex;
        Renderer->Texture2DSlots[Renderer->Texture2DSlotIndex] = Texture2D;
        Renderer->Texture2DSlotIndex++;

        GCRendererGraphicsPipeline_UpdateDescriptors(
            Renderer->GraphicsPipeline, Renderer->GraphicsPipelineDescriptors, Renderer->GraphicsPipelineDescriptorCount
        );
    }

    for (uint32_t Counter = 0; Counter < 4; Counter++)
    {
        const GCVector4 TransformVector = GCMatrix4x4_MultiplyByVector(
            Transform, GCVector4_Create(
                           Renderer->QuadVertexPositions[Counter].X, Renderer->QuadVertexPositions[Counter].Y,
                           Renderer->QuadVertexPositions[Counter].Z, 1.0f
                       )
        );

        Renderer->QuadVertexBufferPointer->Position =
            GCVector3_Create(TransformVector.X, TransformVector.Y, TransformVector.Z);
        Renderer->QuadVertexBufferPointer->Normal = Renderer->QuadVertexNormals[Counter];
        Renderer->QuadVertexBufferPointer->Color = TintColor;
        Renderer->QuadVertexBufferPointer->TextureCoordinate = Renderer->QuadTextureCoordinates[Counter];
        Renderer->QuadVertexBufferPointer->TextureIndex = TextureIndex;
        Renderer->QuadVertexBufferPointer->EntityID = 0;
        Renderer->QuadVertexBufferPointer++;
    }

    Renderer->QuadIndexCount += 6;
}

void GCRenderer_DrawEntity(const GCEntity Entity)
{
    const GCTransformComponent* const TransformComponent = GCEntity_GetTransformComponent(Entity);
    const GCMeshComponent* const MeshComponent = GCEntity_GetMeshComponent(Entity);

    const GCMatrix4x4 Transform = GCTransformComponent_GetTransform(TransformComponent);
    GCRendererMesh* const Mesh = MeshComponent->Mesh;

    if (Renderer->DrawDataCount >= Renderer->MaximumDrawDataCount)
    {
        Renderer->MaximumDrawDataCount += Renderer->MaximumDrawDataCount;
        Renderer->DrawData = (GCRendererDrawData*)GCMemory_Reallocate(
            Renderer->DrawData, Renderer->MaximumDrawDataCount * sizeof(GCRendererDrawData)
        );
    }

    const GCRendererVertex* const OriginalVertices =
        (const GCRendererVertex* const)GCRendererVertexBuffer_GetVertices(Mesh->VertexBuffer);
    const uint32_t VertexCount = GCRendererVertexBuffer_GetVertexCount(Mesh->VertexBuffer);

    GCRendererVertex* Vertices = (GCRendererVertex*)GCMemory_Allocate(VertexCount * sizeof(GCRendererVertex));
    memcpy(Vertices, OriginalVertices, VertexCount * sizeof(GCRendererVertex));

    for (uint32_t Counter = 0; Counter < VertexCount; Counter++)
    {
        const GCVector3 Position = Vertices[Counter].Position;
        const GCVector4 TransformVector =
            GCMatrix4x4_MultiplyByVector(&Transform, GCVector4_Create(Position.X, Position.Y, Position.Z, 1.0f));

        Vertices[Counter].Position = GCVector3_Create(TransformVector.X, TransformVector.Y, TransformVector.Z);
    }

    GCRendererVertexBuffer_SetVertices(Mesh->VertexBuffer, Vertices, VertexCount * sizeof(GCRendererVertex));

    GCMemory_Free(Vertices);

    Renderer->DrawData[Renderer->DrawDataCount].VertexBuffer = Mesh->VertexBuffer;
    Renderer->DrawData[Renderer->DrawDataCount].VertexCount = GCRendererVertexBuffer_GetVertexCount(Mesh->VertexBuffer);
    Renderer->DrawData[Renderer->DrawDataCount].IndexBuffer = Mesh->IndexBuffer;
    Renderer->DrawData[Renderer->DrawDataCount].IndexCount = GCRendererIndexBuffer_GetIndexCount(Mesh->IndexBuffer);

    Renderer->DrawDataCount++;
}

void GCRenderer_EndWorld(void)
{
    GCRenderer_EndBatch();

    for (uint32_t Counter = 0; Counter < Renderer->DrawDataCount; Counter++)
    {
        GCRendererCommandList_BindVertexBuffer(Renderer->CommandList, Renderer->DrawData[Counter].VertexBuffer);
        GCRendererCommandList_BindIndexBuffer(Renderer->CommandList, Renderer->DrawData[Counter].IndexBuffer);
        GCRendererCommandList_DrawIndexed(Renderer->CommandList, Renderer->DrawData[Counter].IndexCount, 0);
    }

    GCRendererCommandList_EndAttachmentRenderPass(Renderer->CommandList, Renderer->Framebuffer);
}

void GCRenderer_BeginImGui(void)
{
    const float ClearColorSwapChain[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    GCRendererCommandList_BeginSwapChainRenderPass(
        Renderer->CommandList, Renderer->GraphicsPipeline, Renderer->Framebuffer, ClearColorSwapChain
    );
}

void GCRenderer_EndImGui(void)
{
    GCRendererCommandList_EndSwapChainRenderPass(Renderer->CommandList);
}

void GCRenderer_Present(void)
{
    GCRendererCommandList_EndRecord(Renderer->CommandList);
    GCRendererCommandList_SubmitAndPresent(Renderer->CommandList);
}

void GCRenderer_Resize(void)
{
    if (Renderer)
    {
        GCRendererCommandList_ShouldSwapChainResize(Renderer->CommandList, true);
    }
}

void GCRenderer_Terminate(void)
{
    GCRendererDevice_WaitIdle(Renderer->Device);

    GCRendererFramebuffer_Destroy(Renderer->Framebuffer);
    GCRendererGraphicsPipeline_Destroy(Renderer->GraphicsPipeline);

    GCRendererIndexBuffer_Destroy(Renderer->QuadIndexBuffer);
    GCRendererVertexBuffer_Destroy(Renderer->QuadVertexBuffer);

    GCRendererShader_Destroy(Renderer->BasicShader);
    GCRendererTexture2D_Destroy(Renderer->WhiteTexture2D);
    GCRendererUniformBuffer_Destroy(Renderer->UniformBuffer);
    GCRendererCommandList_Destroy(Renderer->CommandList);
    GCRendererSwapChain_Destroy(Renderer->SwapChain);
    GCRendererDevice_Destroy(Renderer->Device);

    GCMemory_Free(Renderer->DrawData);
    GCMemory_Free(Renderer->Texture2DSlots);
    GCMemory_Free(Renderer->QuadVertexBufferBase);
    GCMemory_Free(Renderer);
}

GCRendererDevice* const GCRenderer_GetDevice(void)
{
    return Renderer->Device;
}

GCRendererSwapChain* const GCRenderer_GetSwapChain(void)
{
    return Renderer->SwapChain;
}

GCRendererCommandList* const GCRenderer_GetCommandList(void)
{
    return Renderer->CommandList;
}

GCRendererGraphicsPipeline* const GCRenderer_GetGraphicsPipeline(void)
{
    return Renderer->GraphicsPipeline;
}

GCRendererFramebuffer* const GCRenderer_GetFramebuffer(void)
{
    return Renderer->Framebuffer;
}

void GCRenderer_ResizeSwapChain(void)
{
    uint32_t Width = 0, Height = 0;
    GCWindow_GetWindowSize(GCApplication_GetWindow(), &Width, &Height);

    if (Width > 0 && Height > 0)
    {
        GCRendererSwapChain_Recreate(Renderer->SwapChain);
        GCRendererFramebuffer_RecreateSwapChainFramebuffer(Renderer->Framebuffer);
    }
}

void GCRenderer_BeginBatch(void)
{
    Renderer->QuadVertexBufferPointer = Renderer->QuadVertexBufferBase;
    Renderer->QuadIndexCount = 0;
    Renderer->Texture2DSlotIndex = 1;
}

void GCRenderer_EndBatch(void)
{
    const size_t VertexSize = (uint8_t*)Renderer->QuadVertexBufferPointer - (uint8_t*)Renderer->QuadVertexBufferBase;
    GCRendererVertexBuffer_SetVertices(Renderer->QuadVertexBuffer, Renderer->QuadVertexBufferBase, VertexSize);

    GCRendererCommandList_BindVertexBuffer(Renderer->CommandList, Renderer->QuadVertexBuffer);
    GCRendererCommandList_BindIndexBuffer(Renderer->CommandList, Renderer->QuadIndexBuffer);
    GCRendererCommandList_DrawIndexed(Renderer->CommandList, Renderer->QuadIndexCount, 0);
}

void GCRenderer_NextBatch(void)
{
    GCRendererIndexBuffer_Destroy(Renderer->QuadIndexBuffer);
    GCRendererVertexBuffer_Destroy(Renderer->QuadVertexBuffer);

    GCMemory_Free(Renderer->QuadVertexBufferBase);

    Renderer->MaximumQuads *= 2;
    Renderer->MaximumQuadVertices = Renderer->MaximumQuads * 4;
    Renderer->MaximumQuadIndices = Renderer->MaximumQuads * 6;

    GCRendererVertexBufferDescription QuadVertexBufferDescription = {0};
    QuadVertexBufferDescription.Device = Renderer->Device;
    QuadVertexBufferDescription.CommandList = Renderer->CommandList;
    QuadVertexBufferDescription.Vertices = NULL;
    QuadVertexBufferDescription.VertexCount = Renderer->MaximumQuadVertices;
    QuadVertexBufferDescription.VertexSize = Renderer->MaximumQuadVertices * sizeof(GCRendererVertex);
    Renderer->QuadVertexBuffer = GCRendererVertexBuffer_CreateDynamic(&QuadVertexBufferDescription);

    uint32_t* QuadIndices = (uint32_t*)GCMemory_AllocateZero(Renderer->MaximumQuadIndices * sizeof(uint32_t));

    uint32_t QuadIndexOffset = 0;
    for (uint32_t Counter = 0; Counter < Renderer->MaximumQuadIndices; Counter += 6)
    {
        QuadIndices[Counter + 0] = QuadIndexOffset + 0;
        QuadIndices[Counter + 1] = QuadIndexOffset + 1;
        QuadIndices[Counter + 2] = QuadIndexOffset + 2;

        QuadIndices[Counter + 3] = QuadIndexOffset + 2;
        QuadIndices[Counter + 4] = QuadIndexOffset + 3;
        QuadIndices[Counter + 5] = QuadIndexOffset + 0;

        QuadIndexOffset += 4;
    }

    GCRendererIndexBufferDescription QuadIndexBufferDescription = {0};
    QuadIndexBufferDescription.Device = Renderer->Device;
    QuadIndexBufferDescription.CommandList = Renderer->CommandList;
    QuadIndexBufferDescription.Indices = QuadIndices;
    QuadIndexBufferDescription.IndexCount = Renderer->MaximumQuadIndices;
    QuadIndexBufferDescription.IndexSize = Renderer->MaximumQuadIndices * sizeof(uint32_t);
    Renderer->QuadIndexBuffer = GCRendererIndexBuffer_Create(&QuadIndexBufferDescription);

    GCMemory_Free(QuadIndices);

    Renderer->QuadVertexBufferBase =
        (GCRendererVertex*)GCMemory_AllocateZero(Renderer->MaximumQuadVertices * sizeof(GCRendererVertex));
    Renderer->QuadVertexBufferPointer = Renderer->QuadVertexBufferBase;
}
