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

#include "Renderer/Vulkan/VulkanRendererGraphicsPipeline.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererEnums.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanRendererShader.h"
#include "Renderer/Vulkan/VulkanRendererSwapChain.h"
#include "Renderer/Vulkan/VulkanRendererTexture2D.h"
#include "Renderer/Vulkan/VulkanRendererUniformBuffer.h"
#include "Renderer/Vulkan/VulkanUtilities.h"

#include <stdbool.h>
#include <string.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererGraphicsPipeline
{
    const GCRendererDevice* Device;
    const GCRendererSwapChain* SwapChain;
    const GCRendererCommandList* CommandList;
    const GCRendererShader* Shader;

    GCRendererGraphicsPipelineAttachment* Attachments;
    uint32_t AttachmentCount;
    GCRendererSampleCount SampleCount;
    GCRendererGraphicsPipelineVertexInput VertexInput;
    GCRendererGraphicsPipelineDescriptor* Descriptors;
    uint32_t DescriptorCount;

    VkRenderPass SwapChainRenderPassHandle, AttachmentRenderPassHandle;
    VkDescriptorSetLayout DescriptorSetLayoutHandle;
    VkDescriptorPool DescriptorPoolHandle;
    VkDescriptorSet DescriptorSetHandle;
    VkPipelineLayout PipelineLayoutHandle;
    VkPipeline PipelineHandle;
} GCRendererGraphicsPipeline;

static uint32_t GCRendererGraphicsPipeline_GetColorAttachmentCount(
    const GCRendererGraphicsPipeline* const GraphicsPipeline
);
static uint32_t GCRendererGraphicsPipeline_GetColorResolveAttachmentCount(
    const GCRendererGraphicsPipeline* const GraphicsPipeline
);
static uint32_t GCRendererGraphicsPipeline_GetDepthAttachmentCount(
    const GCRendererGraphicsPipeline* const GraphicsPipeline
);
static void GCRendererGraphicsPipeline_CreateSwapChainRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateAttachmentRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateDescriptorSetLayout(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateDescriptorPool(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateDescriptorSets(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_UpdateDescriptorSets(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_DestroyObjects(GCRendererGraphicsPipeline* const GraphicsPipeline);

static VkFormat GCRendererGraphicsPipeline_ToVkFormat(const GCRendererGraphicsPipelineVertexInputAttributeFormat Format
);
static VkDescriptorType GCRendererGraphicsPipeline_ToVkDescriptorType(
    const GCRendererGraphicsPipelineDescriptorType Type
);

GCRendererGraphicsPipeline* GCRendererGraphicsPipeline_Create(
    const GCRendererGraphicsPipelineDescription* const Description
)
{
    GCRendererGraphicsPipeline* GraphicsPipeline =
        (GCRendererGraphicsPipeline*)GCMemory_AllocateZero(sizeof(GCRendererGraphicsPipeline));
    GraphicsPipeline->Device = Description->Device;
    GraphicsPipeline->SwapChain = Description->SwapChain;
    GraphicsPipeline->CommandList = Description->CommandList;
    GraphicsPipeline->Shader = Description->Shader;
    GraphicsPipeline->Attachments = (GCRendererGraphicsPipelineAttachment*)GCMemory_AllocateZero(
        Description->AttachmentCount * sizeof(GCRendererGraphicsPipelineAttachment)
    );
    memcpy(
        GraphicsPipeline->Attachments, Description->Attachments,
        Description->AttachmentCount * sizeof(GCRendererGraphicsPipelineAttachment)
    );
    GraphicsPipeline->AttachmentCount = Description->AttachmentCount;
    GraphicsPipeline->SampleCount = Description->SampleCount;
    GraphicsPipeline->VertexInput = Description->VertexInput;
    GraphicsPipeline->Descriptors = (GCRendererGraphicsPipelineDescriptor*)GCMemory_AllocateZero(
        Description->DescriptorCount * sizeof(GCRendererGraphicsPipelineDescriptor)
    );
    memcpy(
        GraphicsPipeline->Descriptors, Description->Descriptors,
        Description->DescriptorCount * sizeof(GCRendererGraphicsPipelineDescriptor)
    );
    GraphicsPipeline->DescriptorCount = Description->DescriptorCount;

    GCRendererGraphicsPipeline_CreateSwapChainRenderPass(GraphicsPipeline);
    GCRendererGraphicsPipeline_CreateAttachmentRenderPass(GraphicsPipeline);
    GCRendererGraphicsPipeline_CreateDescriptorSetLayout(GraphicsPipeline);
    GCRendererGraphicsPipeline_CreateGraphicsPipeline(GraphicsPipeline);
    GCRendererGraphicsPipeline_CreateDescriptorPool(GraphicsPipeline);
    GCRendererGraphicsPipeline_CreateDescriptorSets(GraphicsPipeline);

    return GraphicsPipeline;
}

void GCRendererGraphicsPipeline_UpdateDescriptors(
    GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererGraphicsPipelineDescriptor* const Descriptors,
    const uint32_t DescriptorCount
)
{
    GC_ASSERT_WITH_MESSAGE(
        DescriptorCount == GraphicsPipeline->DescriptorCount,
        "DescriptorCount (%d) must be the same as GraphicsPipeline->DescriptorCount (%d).", DescriptorCount,
        GraphicsPipeline->DescriptorCount
    );

    memcpy(GraphicsPipeline->Descriptors, Descriptors, DescriptorCount * sizeof(GCRendererGraphicsPipelineDescriptor));
    GCRendererGraphicsPipeline_UpdateDescriptorSets(GraphicsPipeline);
}

void GCRendererGraphicsPipeline_Destroy(GCRendererGraphicsPipeline* GraphicsPipeline)
{
    GCRendererDevice_WaitIdle(GraphicsPipeline->Device);

    GCRendererGraphicsPipeline_DestroyObjects(GraphicsPipeline);

    GCMemory_Free(GraphicsPipeline->Descriptors);
    GCMemory_Free(GraphicsPipeline->Attachments);
    GCMemory_Free(GraphicsPipeline);
}

VkRenderPass GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(
    const GCRendererGraphicsPipeline* const GraphicsPipeline
)
{
    return GraphicsPipeline->SwapChainRenderPassHandle;
}

VkRenderPass GCRendererGraphicsPipeline_GetAttachmentRenderPassHandle(
    const GCRendererGraphicsPipeline* const GraphicsPipeline
)
{
    return GraphicsPipeline->AttachmentRenderPassHandle;
}

VkPipelineLayout GCRendererGraphicsPipeline_GetPipelineLayoutHandle(
    const GCRendererGraphicsPipeline* const GraphicsPipeline
)
{
    return GraphicsPipeline->PipelineLayoutHandle;
}

VkPipeline GCRendererGraphicsPipeline_GetPipelineHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    return GraphicsPipeline->PipelineHandle;
}

VkDescriptorSet GCRendererGraphicsPipeline_GetDescriptorSetHandle(
    const GCRendererGraphicsPipeline* const GraphicsPipeline
)
{
    return GraphicsPipeline->DescriptorSetHandle;
}

uint32_t GCRendererGraphicsPipeline_GetColorAttachmentCount(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    uint32_t Count = 0;
    for (uint32_t Counter = 0; Counter < GraphicsPipeline->AttachmentCount; Counter++)
    {
        if (GraphicsPipeline->Attachments[Counter].Type == GCRendererAttachmentType_Color)
        {
            Count++;
        }
    }

    return Count;
}

uint32_t GCRendererGraphicsPipeline_GetColorResolveAttachmentCount(
    const GCRendererGraphicsPipeline* const GraphicsPipeline
)
{
    uint32_t Count = 0;
    for (uint32_t Counter = 0; Counter < GraphicsPipeline->AttachmentCount; Counter++)
    {
        if (GraphicsPipeline->Attachments[Counter].Type == GCRendererAttachmentType_Color)
        {
            if (GraphicsPipeline->Attachments[Counter].SampleCount > GCRendererSampleCount_1)
            {
                Count++;
            }
        }
    }

    return Count;
}

uint32_t GCRendererGraphicsPipeline_GetDepthAttachmentCount(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    uint32_t Count = 0;
    for (uint32_t Counter = 0; Counter < GraphicsPipeline->AttachmentCount; Counter++)
    {
        if (GraphicsPipeline->Attachments[Counter].Type == GCRendererAttachmentType_DepthStencil)
        {
            Count++;
        }
    }

    return Count;
}

void GCRendererGraphicsPipeline_CreateSwapChainRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    const VkFormat SwapChainFormat = GCRendererSwapChain_GetFormat(GraphicsPipeline->SwapChain);

    VkAttachmentDescription ColorAttachmentDescription = {0};
    ColorAttachmentDescription.format = SwapChainFormat;
    ColorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ColorAttachmentReference = {0};
    ColorAttachmentReference.attachment = 0;
    ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription SubpassDescription = {0};
    SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDescription.colorAttachmentCount = 1;
    SubpassDescription.pColorAttachments = &ColorAttachmentReference;

    VkSubpassDependency SubpassDependency = {0};
    SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependency.dstSubpass = 0;
    SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.srcAccessMask = 0;
    SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo RenderPassInformation = {0};
    RenderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInformation.attachmentCount = 1;
    RenderPassInformation.pAttachments = &ColorAttachmentDescription;
    RenderPassInformation.subpassCount = 1;
    RenderPassInformation.pSubpasses = &SubpassDescription;
    RenderPassInformation.dependencyCount = 1;
    RenderPassInformation.pDependencies = &SubpassDependency;

    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

    GC_VULKAN_VALIDATE(
        vkCreateRenderPass(DeviceHandle, &RenderPassInformation, NULL, &GraphicsPipeline->SwapChainRenderPassHandle),
        "Failed to create a swap chain render pass."
    );
}

static void GCRendererGraphicsPipeline_CreateAttachmentRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    const uint32_t ColorAttachmentCount = GCRendererGraphicsPipeline_GetColorAttachmentCount(GraphicsPipeline);
    const uint32_t ColorResolveAttachmentCount =
        GCRendererGraphicsPipeline_GetColorResolveAttachmentCount(GraphicsPipeline);
    const uint32_t DepthAttachmentCount = GCRendererGraphicsPipeline_GetDepthAttachmentCount(GraphicsPipeline);

    VkAttachmentDescription* AttachmentDescriptions = (VkAttachmentDescription*)GCMemory_AllocateZero(
        (ColorAttachmentCount + ColorResolveAttachmentCount + DepthAttachmentCount) * sizeof(VkAttachmentDescription)
    );
    VkAttachmentReference* ColorAttachmentReferences =
        (VkAttachmentReference*)GCMemory_AllocateZero(ColorAttachmentCount * sizeof(VkAttachmentReference));
    VkAttachmentReference* ColorResolveAttachmentReferences = NULL;

    if (ColorResolveAttachmentCount > 0)
    {
        ColorResolveAttachmentReferences =
            (VkAttachmentReference*)GCMemory_AllocateZero(ColorResolveAttachmentCount * sizeof(VkAttachmentReference));
    }

    VkAttachmentReference DepthAttachmentReference = {0};

    uint32_t LastAttachmentIndex = 0;
    for (uint32_t Counter = 0; Counter < GraphicsPipeline->AttachmentCount; Counter++, LastAttachmentIndex++)
    {
        const GCRendererGraphicsPipelineAttachment Attachment = GraphicsPipeline->Attachments[Counter];

        AttachmentDescriptions[Counter].format =
            GCVulkanUtilities_ToVkFormat(GraphicsPipeline->Device, Attachment.Format);
        AttachmentDescriptions[Counter].samples =
            GCVulkanUtilities_ToVkSampleCountFlagBits(GraphicsPipeline->Device, Attachment.SampleCount);
        AttachmentDescriptions[Counter].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        AttachmentDescriptions[Counter].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        AttachmentDescriptions[Counter].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        AttachmentDescriptions[Counter].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (Attachment.Type == GCRendererAttachmentType_Color)
        {
            AttachmentDescriptions[Counter].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            if (Attachment.SampleCount > GCRendererSampleCount_1)
            {
                AttachmentDescriptions[Counter].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            else
            {
                AttachmentDescriptions[Counter].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            ColorAttachmentReferences[Counter].attachment = Counter;
            ColorAttachmentReferences[Counter].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        else
        {
            AttachmentDescriptions[Counter].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            AttachmentDescriptions[Counter].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            DepthAttachmentReference.attachment = Counter;
            DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
    }

    for (uint32_t Counter = 0; Counter < GraphicsPipeline->AttachmentCount; Counter++, LastAttachmentIndex++)
    {
        const GCRendererGraphicsPipelineAttachment Attachment = GraphicsPipeline->Attachments[Counter];

        if (Attachment.Type == GCRendererAttachmentType_Color && Attachment.SampleCount > GCRendererSampleCount_1)
        {
            AttachmentDescriptions[LastAttachmentIndex].format =
                GCVulkanUtilities_ToVkFormat(GraphicsPipeline->Device, Attachment.Format);
            AttachmentDescriptions[LastAttachmentIndex].samples = VK_SAMPLE_COUNT_1_BIT;
            AttachmentDescriptions[LastAttachmentIndex].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            AttachmentDescriptions[LastAttachmentIndex].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            AttachmentDescriptions[LastAttachmentIndex].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            AttachmentDescriptions[LastAttachmentIndex].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            AttachmentDescriptions[LastAttachmentIndex].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            AttachmentDescriptions[LastAttachmentIndex].finalLayout =
                AttachmentDescriptions[Counter].format != VK_FORMAT_R32_UINT ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                                             : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            ColorResolveAttachmentReferences[Counter].attachment = LastAttachmentIndex;
            ColorResolveAttachmentReferences[Counter].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }

    VkSubpassDescription SubpassDescription = {0};
    SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDescription.colorAttachmentCount = ColorAttachmentCount;
    SubpassDescription.pColorAttachments = ColorAttachmentReferences;
    SubpassDescription.pDepthStencilAttachment = &DepthAttachmentReference;
    SubpassDescription.pResolveAttachments = ColorResolveAttachmentReferences;

    VkSubpassDependency SubpassDependency = {0};
    SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependency.dstSubpass = 0;
    SubpassDependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    SubpassDependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    SubpassDependency.srcAccessMask = 0;
    SubpassDependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo RenderPassInformation = {0};
    RenderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInformation.attachmentCount = ColorAttachmentCount + ColorResolveAttachmentCount + DepthAttachmentCount;
    RenderPassInformation.pAttachments = AttachmentDescriptions;
    RenderPassInformation.subpassCount = 1;
    RenderPassInformation.pSubpasses = &SubpassDescription;
    RenderPassInformation.dependencyCount = 1;
    RenderPassInformation.pDependencies = &SubpassDependency;

    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

    GC_VULKAN_VALIDATE(
        vkCreateRenderPass(DeviceHandle, &RenderPassInformation, NULL, &GraphicsPipeline->AttachmentRenderPassHandle),
        "Failed to create an attachment render pass."
    );

    if (ColorResolveAttachmentCount > 0)
    {
        GCMemory_Free(ColorResolveAttachmentReferences);
    }

    GCMemory_Free(ColorAttachmentReferences);
    GCMemory_Free(AttachmentDescriptions);
}

void GCRendererGraphicsPipeline_CreateDescriptorSetLayout(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

    VkDescriptorSetLayoutBinding* DescriptorSetLayoutBindings = (VkDescriptorSetLayoutBinding*)GCMemory_AllocateZero(
        GraphicsPipeline->DescriptorCount * sizeof(VkDescriptorSetLayoutBinding)
    );

    for (uint32_t Counter = 0; Counter < GraphicsPipeline->DescriptorCount; Counter++)
    {
        const GCRendererGraphicsPipelineDescriptor Descriptor = GraphicsPipeline->Descriptors[Counter];

        DescriptorSetLayoutBindings[Counter].binding = Counter;
        DescriptorSetLayoutBindings[Counter].descriptorType =
            GCRendererGraphicsPipeline_ToVkDescriptorType(Descriptor.Type);
        DescriptorSetLayoutBindings[Counter].descriptorCount = Descriptor.DescriptorCount;
        DescriptorSetLayoutBindings[Counter].stageFlags =
            Descriptor.Type == GCRendererGraphicsPipelineDescriptorType_UniformBuffer ? VK_SHADER_STAGE_VERTEX_BIT
                                                                                      : VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutInformation = {0};
    DescriptorSetLayoutInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DescriptorSetLayoutInformation.bindingCount = GraphicsPipeline->DescriptorCount;
    DescriptorSetLayoutInformation.pBindings = DescriptorSetLayoutBindings;

    GC_VULKAN_VALIDATE(
        vkCreateDescriptorSetLayout(
            DeviceHandle, &DescriptorSetLayoutInformation, NULL, &GraphicsPipeline->DescriptorSetLayoutHandle
        ),
        "Failed to create a descriptor set layout."
    );

    GCMemory_Free(DescriptorSetLayoutBindings);
}

void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

    VkPipelineLayoutCreateInfo PipelineLayoutInformation = {0};
    PipelineLayoutInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInformation.setLayoutCount = 1;
    PipelineLayoutInformation.pSetLayouts = &GraphicsPipeline->DescriptorSetLayoutHandle;

    GC_VULKAN_VALIDATE(
        vkCreatePipelineLayout(DeviceHandle, &PipelineLayoutInformation, NULL, &GraphicsPipeline->PipelineLayoutHandle),
        "Failed to create a pipeline layout."
    );

    VkPipelineShaderStageCreateInfo PipelineVertexShaderStageInformation = {0};
    PipelineVertexShaderStageInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineVertexShaderStageInformation.stage = VK_SHADER_STAGE_VERTEX_BIT;
    PipelineVertexShaderStageInformation.module =
        GCRendererShader_GetVertexShaderModuleHandle(GraphicsPipeline->Shader);
    PipelineVertexShaderStageInformation.pName = "main";

    VkPipelineShaderStageCreateInfo PipelineFragmentShaderStageInformation = {0};
    PipelineFragmentShaderStageInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineFragmentShaderStageInformation.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    PipelineFragmentShaderStageInformation.module =
        GCRendererShader_GetFragmentShaderModuleHandle(GraphicsPipeline->Shader);
    PipelineFragmentShaderStageInformation.pName = "main";

    const VkPipelineShaderStageCreateInfo PipelineShaderStageInformation[2] = {
        PipelineVertexShaderStageInformation, PipelineFragmentShaderStageInformation};

    VkVertexInputBindingDescription* VertexInputBindingDescriptions = (VkVertexInputBindingDescription*)
        GCMemory_AllocateZero(GraphicsPipeline->VertexInput.BindingCount * sizeof(VkVertexInputBindingDescription));

    for (uint32_t Counter = 0; Counter < GraphicsPipeline->VertexInput.BindingCount; Counter++)
    {
        VertexInputBindingDescriptions[Counter].binding = GraphicsPipeline->VertexInput.Bindings[Counter].Binding;
        VertexInputBindingDescriptions[Counter].stride = GraphicsPipeline->VertexInput.Bindings[Counter].Stride;
        VertexInputBindingDescriptions[Counter].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }

    VkVertexInputAttributeDescription* VertexInputAttributeDescriptions = (VkVertexInputAttributeDescription*)
        GCMemory_AllocateZero(GraphicsPipeline->VertexInput.AttributeCount * sizeof(VkVertexInputAttributeDescription));

    for (uint32_t Counter = 0; Counter < GraphicsPipeline->VertexInput.AttributeCount; Counter++)
    {
        VertexInputAttributeDescriptions[Counter].location = GraphicsPipeline->VertexInput.Attributes[Counter].Location;
        VertexInputAttributeDescriptions[Counter].binding = 0;
        VertexInputAttributeDescriptions[Counter].format =
            GCRendererGraphicsPipeline_ToVkFormat(GraphicsPipeline->VertexInput.Attributes[Counter].Format);
        VertexInputAttributeDescriptions[Counter].offset = GraphicsPipeline->VertexInput.Attributes[Counter].Offset;
    }

    VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateInformation = {0};
    PipelineVertexInputStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    PipelineVertexInputStateInformation.vertexBindingDescriptionCount = GraphicsPipeline->VertexInput.BindingCount;
    PipelineVertexInputStateInformation.pVertexBindingDescriptions = VertexInputBindingDescriptions;
    PipelineVertexInputStateInformation.vertexAttributeDescriptionCount = GraphicsPipeline->VertexInput.AttributeCount;
    PipelineVertexInputStateInformation.pVertexAttributeDescriptions = VertexInputAttributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateInformation = {0};
    PipelineInputAssemblyStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    PipelineInputAssemblyStateInformation.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PipelineInputAssemblyStateInformation.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo PipelineViewportStateInformation = {0};
    PipelineViewportStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    PipelineViewportStateInformation.viewportCount = 1;
    PipelineViewportStateInformation.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateInformation = {0};
    PipelineRasterizationStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    PipelineRasterizationStateInformation.depthClampEnable = VK_FALSE;
    PipelineRasterizationStateInformation.rasterizerDiscardEnable = VK_FALSE;
    PipelineRasterizationStateInformation.polygonMode = VK_POLYGON_MODE_FILL;
    PipelineRasterizationStateInformation.cullMode = VK_CULL_MODE_NONE;
    PipelineRasterizationStateInformation.frontFace = VK_FRONT_FACE_CLOCKWISE;
    PipelineRasterizationStateInformation.depthBiasEnable = VK_FALSE;
    PipelineRasterizationStateInformation.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateInformation = {0};
    PipelineMultisampleStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    PipelineMultisampleStateInformation.rasterizationSamples =
        GCVulkanUtilities_ToVkSampleCountFlagBits(GraphicsPipeline->Device, GraphicsPipeline->SampleCount);
    PipelineMultisampleStateInformation.sampleShadingEnable = VK_FALSE;
    PipelineMultisampleStateInformation.minSampleShading = 1.0f;

    VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateInformation = {0};
    PipelineDepthStencilStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    PipelineDepthStencilStateInformation.depthTestEnable = VK_TRUE;
    PipelineDepthStencilStateInformation.depthWriteEnable = VK_TRUE;
    PipelineDepthStencilStateInformation.depthCompareOp = VK_COMPARE_OP_LESS;
    PipelineDepthStencilStateInformation.depthBoundsTestEnable = VK_FALSE;
    PipelineDepthStencilStateInformation.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentStates[2] = {0};
    PipelineColorBlendAttachmentStates[0].blendEnable = VK_TRUE;
    PipelineColorBlendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    PipelineColorBlendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    PipelineColorBlendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
    PipelineColorBlendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    PipelineColorBlendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    PipelineColorBlendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
    PipelineColorBlendAttachmentStates[0].colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    PipelineColorBlendAttachmentStates[1].blendEnable = VK_FALSE;
    PipelineColorBlendAttachmentStates[1].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    PipelineColorBlendAttachmentStates[1].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    PipelineColorBlendAttachmentStates[1].colorBlendOp = VK_BLEND_OP_ADD;
    PipelineColorBlendAttachmentStates[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    PipelineColorBlendAttachmentStates[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    PipelineColorBlendAttachmentStates[1].alphaBlendOp = VK_BLEND_OP_ADD;
    PipelineColorBlendAttachmentStates[1].colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateInformation = {0};
    PipelineColorBlendStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    PipelineColorBlendStateInformation.logicOpEnable = VK_FALSE;
    PipelineColorBlendStateInformation.attachmentCount = 2;
    PipelineColorBlendStateInformation.pAttachments = PipelineColorBlendAttachmentStates;

    const VkDynamicState DynamicStates[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo PipelineDynamicStateInformation = {0};
    PipelineDynamicStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    PipelineDynamicStateInformation.dynamicStateCount = 2;
    PipelineDynamicStateInformation.pDynamicStates = DynamicStates;

    VkGraphicsPipelineCreateInfo GraphicsPipelineInformation = {0};
    GraphicsPipelineInformation.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    GraphicsPipelineInformation.stageCount = 2;
    GraphicsPipelineInformation.pStages = PipelineShaderStageInformation;
    GraphicsPipelineInformation.pVertexInputState = &PipelineVertexInputStateInformation;
    GraphicsPipelineInformation.pInputAssemblyState = &PipelineInputAssemblyStateInformation;
    GraphicsPipelineInformation.pViewportState = &PipelineViewportStateInformation;
    GraphicsPipelineInformation.pRasterizationState = &PipelineRasterizationStateInformation;
    GraphicsPipelineInformation.pMultisampleState = &PipelineMultisampleStateInformation;
    GraphicsPipelineInformation.pDepthStencilState = &PipelineDepthStencilStateInformation;
    GraphicsPipelineInformation.pColorBlendState = &PipelineColorBlendStateInformation;
    GraphicsPipelineInformation.pDynamicState = &PipelineDynamicStateInformation;
    GraphicsPipelineInformation.layout = GraphicsPipeline->PipelineLayoutHandle;
    GraphicsPipelineInformation.renderPass = GraphicsPipeline->AttachmentRenderPassHandle;
    GraphicsPipelineInformation.subpass = 0;

    GC_VULKAN_VALIDATE(
        vkCreateGraphicsPipelines(
            DeviceHandle, VK_NULL_HANDLE, 1, &GraphicsPipelineInformation, NULL, &GraphicsPipeline->PipelineHandle
        ),
        "Failed to create a graphics pipeline."
    );

    GCMemory_Free(VertexInputAttributeDescriptions);
    GCMemory_Free(VertexInputBindingDescriptions);
}

void GCRendererGraphicsPipeline_CreateDescriptorPool(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

    VkDescriptorPoolSize* DescriptorPoolSizes =
        (VkDescriptorPoolSize*)GCMemory_AllocateZero(GraphicsPipeline->DescriptorCount * sizeof(VkDescriptorPoolSize));

    for (uint32_t Counter = 0; Counter < GraphicsPipeline->DescriptorCount; Counter++)
    {
        const GCRendererGraphicsPipelineDescriptor Descriptor = GraphicsPipeline->Descriptors[Counter];

        DescriptorPoolSizes[Counter].type = GCRendererGraphicsPipeline_ToVkDescriptorType(Descriptor.Type);
        DescriptorPoolSizes[Counter].descriptorCount = Descriptor.DescriptorCount;
    }

    VkDescriptorPoolCreateInfo DescriptorPoolInformation = {0};
    DescriptorPoolInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    DescriptorPoolInformation.maxSets = 1;
    DescriptorPoolInformation.poolSizeCount = GraphicsPipeline->DescriptorCount;
    DescriptorPoolInformation.pPoolSizes = DescriptorPoolSizes;

    GC_VULKAN_VALIDATE(
        vkCreateDescriptorPool(DeviceHandle, &DescriptorPoolInformation, NULL, &GraphicsPipeline->DescriptorPoolHandle),
        "Failed to create a descriptor pool."
    );

    GCMemory_Free(DescriptorPoolSizes);
}

void GCRendererGraphicsPipeline_CreateDescriptorSets(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

    VkDescriptorSetAllocateInfo DescriptorSetAllocateInformation = {0};
    DescriptorSetAllocateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    DescriptorSetAllocateInformation.descriptorPool = GraphicsPipeline->DescriptorPoolHandle;
    DescriptorSetAllocateInformation.descriptorSetCount = 1;
    DescriptorSetAllocateInformation.pSetLayouts = &GraphicsPipeline->DescriptorSetLayoutHandle;

    GC_VULKAN_VALIDATE(
        vkAllocateDescriptorSets(
            DeviceHandle, &DescriptorSetAllocateInformation, &GraphicsPipeline->DescriptorSetHandle
        ),
        "Failed to allocate a descriptor set."
    );

    GCRendererGraphicsPipeline_UpdateDescriptorSets(GraphicsPipeline);
}

void GCRendererGraphicsPipeline_UpdateDescriptorSets(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

    VkWriteDescriptorSet* WriteDescriptorSets =
        (VkWriteDescriptorSet*)GCMemory_AllocateZero(GraphicsPipeline->DescriptorCount * sizeof(VkWriteDescriptorSet));

    VkDescriptorBufferInfo DescriptorBufferInformation = {0};
    VkDescriptorImageInfo DescriptorImageInformation = {0};
    VkDescriptorImageInfo* DescriptorImageArrayInformation = NULL;

    for (uint32_t Counter = 0; Counter < GraphicsPipeline->DescriptorCount; Counter++)
    {
        const GCRendererGraphicsPipelineDescriptor Descriptor = GraphicsPipeline->Descriptors[Counter];

        if (Descriptor.Type == GCRendererGraphicsPipelineDescriptorType_UniformBuffer)
        {
            DescriptorBufferInformation.buffer = GCRendererUniformBuffer_GetBufferHandle(Descriptor.UniformBuffer);
            DescriptorBufferInformation.offset = 0;
            DescriptorBufferInformation.range = GCRendererUniformBuffer_GetDataSize(Descriptor.UniformBuffer);
        }
        else if (Descriptor.Type == GCRendererGraphicsPipelineDescriptorType_Texture)
        {
            DescriptorImageInformation.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            DescriptorImageInformation.imageView = GCRendererTexture2D_GetImageViewHandle(Descriptor.Texture2D);
            DescriptorImageInformation.sampler = GCRendererTexture2D_GetSamplerHandle(Descriptor.Texture2D);
        }
        else if (Descriptor.Type == GCRendererGraphicsPipelineDescriptorType_TextureArray)
        {
            DescriptorImageArrayInformation = (VkDescriptorImageInfo*)GCMemory_AllocateZero(
                Descriptor.DescriptorCount * sizeof(VkDescriptorImageInfo)
            );

            for (uint32_t Counter2 = 0; Counter2 < Descriptor.DescriptorCount; Counter2++)
            {
                DescriptorImageArrayInformation[Counter2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                DescriptorImageArrayInformation[Counter2].imageView =
                    GCRendererTexture2D_GetImageViewHandle(Descriptor.Texture2DArray[Counter2]);
                DescriptorImageArrayInformation[Counter2].sampler =
                    GCRendererTexture2D_GetSamplerHandle(Descriptor.Texture2DArray[Counter2]);
            }
        }

        WriteDescriptorSets[Counter].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteDescriptorSets[Counter].dstSet = GraphicsPipeline->DescriptorSetHandle;
        WriteDescriptorSets[Counter].dstBinding = Counter;
        WriteDescriptorSets[Counter].dstArrayElement = 0;
        WriteDescriptorSets[Counter].descriptorCount = Descriptor.DescriptorCount;
        WriteDescriptorSets[Counter].descriptorType = GCRendererGraphicsPipeline_ToVkDescriptorType(Descriptor.Type);

        if (Descriptor.Type == GCRendererGraphicsPipelineDescriptorType_UniformBuffer)
        {
            WriteDescriptorSets[Counter].pBufferInfo = &DescriptorBufferInformation;
        }
        else if (Descriptor.Type == GCRendererGraphicsPipelineDescriptorType_Texture)
        {
            WriteDescriptorSets[Counter].pImageInfo = &DescriptorImageInformation;
        }
        else if (Descriptor.Type == GCRendererGraphicsPipelineDescriptorType_TextureArray)
        {
            WriteDescriptorSets[Counter].pImageInfo = DescriptorImageArrayInformation;
        }
    }

    vkUpdateDescriptorSets(DeviceHandle, GraphicsPipeline->DescriptorCount, WriteDescriptorSets, 0, NULL);

    if (DescriptorImageArrayInformation)
    {
        GCMemory_Free(DescriptorImageArrayInformation);
    }

    GCMemory_Free(WriteDescriptorSets);
}

void GCRendererGraphicsPipeline_DestroyObjects(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

    vkDestroyDescriptorPool(DeviceHandle, GraphicsPipeline->DescriptorPoolHandle, NULL);

    vkDestroyPipeline(DeviceHandle, GraphicsPipeline->PipelineHandle, NULL);
    vkDestroyPipelineLayout(DeviceHandle, GraphicsPipeline->PipelineLayoutHandle, NULL);
    vkDestroyDescriptorSetLayout(DeviceHandle, GraphicsPipeline->DescriptorSetLayoutHandle, NULL);
    vkDestroyRenderPass(DeviceHandle, GraphicsPipeline->SwapChainRenderPassHandle, NULL);
    vkDestroyRenderPass(DeviceHandle, GraphicsPipeline->AttachmentRenderPassHandle, NULL);
}

VkFormat GCRendererGraphicsPipeline_ToVkFormat(const GCRendererGraphicsPipelineVertexInputAttributeFormat Format)
{
    switch (Format)
    {
    case GCRendererGraphicsPipelineVertexInputAttributeFormat_Float: {
        return VK_FORMAT_R32_SFLOAT;

        break;
    }
    case GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector2: {
        return VK_FORMAT_R32G32_SFLOAT;

        break;
    }
    case GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3: {
        return VK_FORMAT_R32G32B32_SFLOAT;

        break;
    }
    case GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector4: {
        return VK_FORMAT_R32G32B32A32_SFLOAT;

        break;
    }
    case GCRendererGraphicsPipelineVertexInputAttributeFormat_UnsignedInteger: {
        return VK_FORMAT_R32_UINT;

        break;
    }
    }

    GC_ASSERT_WITH_MESSAGE(false, "'%d': Invalid GCRendererGraphicsPipelineVertexInputAttributeFormat", Format);
    return VK_FORMAT_UNDEFINED;
}

VkDescriptorType GCRendererGraphicsPipeline_ToVkDescriptorType(const GCRendererGraphicsPipelineDescriptorType Type)
{
    switch (Type)
    {
    case GCRendererGraphicsPipelineDescriptorType_UniformBuffer: {
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        break;
    }
    case GCRendererGraphicsPipelineDescriptorType_Texture: {
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
    }
    case GCRendererGraphicsPipelineDescriptorType_TextureArray: {
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
    }
    }

    GC_ASSERT_WITH_MESSAGE(false, "'%d': Invalid GCRendererGraphicsPipelineDescriptorType.", Type);
    return (VkDescriptorType)-1;
}
