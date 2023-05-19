#define _CRT_SECURE_NO_WARNINGS
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererShader.h"
#include "Core/Memory/Allocator.h"
#include "Core/FileSystem.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.h>

typedef enum GCRendererShaderType
{
	GCRendererShaderType_Vertex,
	GCRendererShaderType_Fragment
} GCRendererShaderType;

typedef struct GCRendererShader
{
	const GCRendererDevice* Device;

	VkShaderModule VertexShaderModuleHandle, FragmentShaderModuleHandle;

	const char* ShaderCacheDirectory;
	uint32_t* VertexShaderData;
	size_t VertexShaderDataSize;
	uint32_t* FragmentShaderData;
	size_t FragmentShaderDataSize;
} GCRendererShader;

VkShaderModule GCRendererShader_GetVertexShaderModuleHandle(const GCRendererShader* const Shader);
VkShaderModule GCRendererShader_GetFragmentShaderModuleHandle(const GCRendererShader* const Shader);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);

static void GCRendererShader_CreateCacheDirectoryIfNeeded(const GCRendererShader* const Shader);
static void GCRendererShader_WriteShaderBinaryFile(const char* const Path, const char* const Data, const size_t Size);
static char* GCRendererShader_ReadShaderSourceFile(const char* const Path);
static size_t GCRendererShader_ReadShaderBinaryFile(const char* const Path, char** Data);
static shaderc_compilation_result_t GCRendererShader_CompileShader(const shaderc_compiler_t Compiler, const shaderc_compile_options_t CompileOptions, const char* const Path, const char* const Source, const GCRendererShaderType Type);
static char* GCRendererShader_GetShaderName(const char* const Path);
static char* GCRendererShader_GetShaderCachePath(const GCRendererShader* const Shader, const char* const Path, const GCRendererShaderType Type);
static void GCRendererShader_CompileOrGetBinaries(GCRendererShader* const Shader, const char* const VertexShaderPath, const char* const FragmentShaderPath);
static void GCRendererShader_CreateShaderModule(GCRendererShader* const Shader);
static void GCRendererShader_DestroyObjects(GCRendererShader* const Shader);

GCRendererShader* GCRendererShader_Create(const GCRendererDevice* const Device, const char* const VertexShaderPath, const char* const FragmentShaderPath)
{
	GCRendererShader* Shader = (GCRendererShader*)GCMemory_Allocate(sizeof(GCRendererShader));
	Shader->Device = Device;
	Shader->VertexShaderModuleHandle = VK_NULL_HANDLE;
	Shader->FragmentShaderModuleHandle = VK_NULL_HANDLE;
	Shader->ShaderCacheDirectory = "Assets/Cache/Shader/Vulkan/";
	Shader->VertexShaderData = NULL;
	Shader->VertexShaderDataSize = 0;
	Shader->FragmentShaderData = NULL;
	Shader->FragmentShaderDataSize = 0;

	GCRendererShader_CreateCacheDirectoryIfNeeded(Shader);
	GCRendererShader_CompileOrGetBinaries(Shader, VertexShaderPath, FragmentShaderPath);
	GCRendererShader_CreateShaderModule(Shader);

	return Shader;
}

void GCRendererShader_Destroy(GCRendererShader* Shader)
{
	GCRendererShader_DestroyObjects(Shader);

	GCMemory_Free(Shader);
}

VkShaderModule GCRendererShader_GetVertexShaderModuleHandle(const GCRendererShader* const Shader)
{
	return Shader->VertexShaderModuleHandle;
}

VkShaderModule GCRendererShader_GetFragmentShaderModuleHandle(const GCRendererShader* const Shader)
{
	return Shader->FragmentShaderModuleHandle;
}

void GCRendererShader_CreateCacheDirectoryIfNeeded(const GCRendererShader* const Shader)
{
	if (!GCFileSystem_Exists(Shader->ShaderCacheDirectory))
	{
		GCFileSystem_CreateDirectories(Shader->ShaderCacheDirectory);
	}
}

void GCRendererShader_WriteShaderBinaryFile(const char* const Path, const char* const Data, const size_t Size)
{
	FILE* ShaderCacheFile = fopen(Path, "wb");

	if (ShaderCacheFile)
	{
		fwrite(Data, 1, Size, ShaderCacheFile);

		fclose(ShaderCacheFile);
	}
}

char* GCRendererShader_ReadShaderSourceFile(const char* const Path)
{
	FILE* ShaderFile = fopen(Path, "r");
	char* ShaderFileSource = NULL;

	if (ShaderFile)
	{
		fseek(ShaderFile, 0, SEEK_END);
		const size_t ShaderFileLength = (size_t)ftell(ShaderFile);
		fseek(ShaderFile, 0, SEEK_SET);

		ShaderFileSource = (char*)GCMemory_AllocateZero(ShaderFileLength * sizeof(char));
		fread(ShaderFileSource, 1, ShaderFileLength, ShaderFile);

		fclose(ShaderFile);
	}

	return ShaderFileSource;
}

size_t GCRendererShader_ReadShaderBinaryFile(const char* const Path, char** Data)
{
	FILE* ShaderCacheFile = fopen(Path, "rb");
	size_t ShaderCacheFileLength = 0;

	if (ShaderCacheFile)
	{
		fseek(ShaderCacheFile, 0, SEEK_END);
		ShaderCacheFileLength = ftell(ShaderCacheFile);
		fseek(ShaderCacheFile, 0, SEEK_SET);

		*Data = GCMemory_Allocate(ShaderCacheFileLength * sizeof(char));
		fread(*Data, 1, ShaderCacheFileLength, ShaderCacheFile);

		fclose(ShaderCacheFile);
	}

	return ShaderCacheFileLength;
}

shaderc_compilation_result_t GCRendererShader_CompileShader(const shaderc_compiler_t Compiler, const shaderc_compile_options_t CompileOptions, const char* const Path, const char* const Source, const GCRendererShaderType Type)
{
	const shaderc_compilation_result_t ShaderCompilationResult = shaderc_compile_into_spv(Compiler, Source, strlen(Source) * sizeof(char), Type == GCRendererShaderType_Vertex ? shaderc_vertex_shader : shaderc_fragment_shader, Path, "main", CompileOptions);
	const shaderc_compilation_status ShaderCompilationStatus = shaderc_result_get_compilation_status(ShaderCompilationResult);

	if (ShaderCompilationStatus != shaderc_compilation_status_success)
	{
		GC_ASSERT_WITH_MESSAGE(false, "Failed to compile GLSL %s shader:\n%s", Type == GCRendererShaderType_Vertex ? "vertex" : "fragment", shaderc_result_get_error_message(ShaderCompilationResult));
	}

	return ShaderCompilationResult;
}

char* GCRendererShader_GetShaderName(const char* const Path)
{
	char* ShaderFileName = GCFileSystem_GetFileName(Path);
	return strtok(ShaderFileName, ".");
}

char* GCRendererShader_GetShaderCachePath(const GCRendererShader* const Shader, const char* const Path, const GCRendererShaderType Type)
{
	char* ShaderName = GCRendererShader_GetShaderName(Path);
	const char* const ShaderFileExtension = Type == GCRendererShaderType_Vertex ? ".cached.vert" : ".cached.frag";
	char* ShaderCachePath = (char*)GCMemory_Allocate((strlen(Shader->ShaderCacheDirectory) + strlen(ShaderName) + strlen(ShaderFileExtension) + 1) * sizeof(char));

	strcpy(ShaderCachePath, Shader->ShaderCacheDirectory);
	strcat(ShaderCachePath, ShaderName);
	strcat(ShaderCachePath, ShaderFileExtension);

	GCMemory_Free(ShaderName);

	return ShaderCachePath;
}

void GCRendererShader_CompileOrGetBinaries(GCRendererShader* const Shader, const char* const VertexShaderPath, const char* const FragmentShaderPath)
{
	char* VertexShaderCachePath = GCRendererShader_GetShaderCachePath(Shader, VertexShaderPath, GCRendererShaderType_Vertex);
	char* FragmentShaderCachePath = GCRendererShader_GetShaderCachePath(Shader, FragmentShaderPath, GCRendererShaderType_Fragment);

	if (GCFileSystem_Exists(VertexShaderCachePath) && GCFileSystem_Exists(FragmentShaderCachePath))
	{
		const GCFileSystemFileAttributes VertexShaderFileAttributes = GCFileSystem_GetFileAttributes(VertexShaderPath);
		const GCFileSystemFileAttributes VertexShaderCacheFileAttributes = GCFileSystem_GetFileAttributes(VertexShaderCachePath);
		const GCFileSystemFileAttributes FragmentShaderFileAttributes = GCFileSystem_GetFileAttributes(FragmentShaderPath);
		const GCFileSystemFileAttributes FragmentShaderCacheFileAttributes = GCFileSystem_GetFileAttributes(FragmentShaderCachePath);

		const GCFileSystemFileTime VertexShaderFileTime = VertexShaderFileAttributes.LastWriteTime;
		const GCFileSystemFileTime VertexShaderCacheFileTime = VertexShaderCacheFileAttributes.LastWriteTime;
		const GCFileSystemFileTime FragmentShaderFileTime = FragmentShaderFileAttributes.LastWriteTime;
		const GCFileSystemFileTime FragmentShaderCacheFileTime = FragmentShaderCacheFileAttributes.LastWriteTime;

		if (!GCFileSystemFileTime_IsNewer(VertexShaderFileTime, VertexShaderCacheFileTime) && !GCFileSystemFileTime_IsNewer(FragmentShaderFileTime, FragmentShaderCacheFileTime))
		{
			char* VertexShaderCacheData = NULL;
			char* FragmentShaderCacheData = NULL;

			Shader->VertexShaderDataSize = GCRendererShader_ReadShaderBinaryFile(VertexShaderCachePath, &VertexShaderCacheData);
			Shader->FragmentShaderDataSize = GCRendererShader_ReadShaderBinaryFile(FragmentShaderCachePath, &FragmentShaderCacheData);

			if (VertexShaderCacheData && FragmentShaderCacheData)
			{
				Shader->VertexShaderData = (uint32_t*)VertexShaderCacheData;
				Shader->FragmentShaderData = (uint32_t*)FragmentShaderCacheData;
			}
		}
	}

	if(!Shader->VertexShaderData || !Shader->FragmentShaderData)
	{
		char* VertexShaderSource = GCRendererShader_ReadShaderSourceFile(VertexShaderPath);
		char* FragmentShaderSource = GCRendererShader_ReadShaderSourceFile(FragmentShaderPath);

		shaderc_compiler_t ShaderCompiler = shaderc_compiler_initialize();
		shaderc_compile_options_t ShaderCompileOptions = shaderc_compile_options_initialize();

		shaderc_compile_options_set_target_env(ShaderCompileOptions, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);

#ifdef GC_BUILD_TYPE_DISTRIBUTION
		shaderc_compile_options_set_optimization_level(ShaderCompileOptions, shaderc_optimization_level_performance);
#endif

		char* VertexShaderCacheData = NULL;
		char* FragmentShaderCacheData = NULL;

		const shaderc_compilation_result_t VertexShaderCompilationResult = GCRendererShader_CompileShader(ShaderCompiler, ShaderCompileOptions, VertexShaderPath, VertexShaderSource, GCRendererShaderType_Vertex);
		VertexShaderCacheData = (char*)shaderc_result_get_bytes(VertexShaderCompilationResult);
		Shader->VertexShaderDataSize = shaderc_result_get_length(VertexShaderCompilationResult);

		const shaderc_compilation_result_t FragmentShaderCompilationResult = GCRendererShader_CompileShader(ShaderCompiler, ShaderCompileOptions, FragmentShaderPath, FragmentShaderSource, GCRendererShaderType_Fragment);
		FragmentShaderCacheData = (char*)shaderc_result_get_bytes(FragmentShaderCompilationResult);
		Shader->FragmentShaderDataSize = shaderc_result_get_length(FragmentShaderCompilationResult);

		GCRendererShader_WriteShaderBinaryFile(VertexShaderCachePath, VertexShaderCacheData, Shader->VertexShaderDataSize);
		GCRendererShader_WriteShaderBinaryFile(FragmentShaderCachePath, FragmentShaderCacheData, Shader->FragmentShaderDataSize);

		Shader->VertexShaderData = (uint32_t*)GCMemory_Allocate(Shader->VertexShaderDataSize);
		Shader->FragmentShaderData = (uint32_t*)GCMemory_Allocate(Shader->FragmentShaderDataSize);

		memcpy(Shader->VertexShaderData, VertexShaderCacheData, Shader->VertexShaderDataSize);
		memcpy(Shader->FragmentShaderData, FragmentShaderCacheData, Shader->FragmentShaderDataSize);

		shaderc_result_release(FragmentShaderCompilationResult);
		shaderc_result_release(VertexShaderCompilationResult);
		shaderc_compile_options_release(ShaderCompileOptions);
		shaderc_compiler_release(ShaderCompiler);

		GCMemory_Free(FragmentShaderSource);
		GCMemory_Free(VertexShaderSource);
	}

	GCMemory_Free(FragmentShaderCachePath);
	GCMemory_Free(VertexShaderCachePath);
}

void GCRendererShader_CreateShaderModule(GCRendererShader* const Shader)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Shader->Device);

	VkShaderModuleCreateInfo VertexShaderModuleInformation = { 0 };
	VertexShaderModuleInformation.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	VertexShaderModuleInformation.codeSize = Shader->VertexShaderDataSize;
	VertexShaderModuleInformation.pCode = Shader->VertexShaderData;

	VkShaderModuleCreateInfo FragmentShaderModuleInformation = { 0 };
	FragmentShaderModuleInformation.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	FragmentShaderModuleInformation.codeSize = Shader->FragmentShaderDataSize;
	FragmentShaderModuleInformation.pCode = Shader->FragmentShaderData;

	GC_VULKAN_VALIDATE(vkCreateShaderModule(DeviceHandle, &VertexShaderModuleInformation, NULL, &Shader->VertexShaderModuleHandle), "Failed to create a Vulkan vertex shader module");
	GC_VULKAN_VALIDATE(vkCreateShaderModule(DeviceHandle, &FragmentShaderModuleInformation, NULL, &Shader->FragmentShaderModuleHandle), "Failed to create a Vulkan fragment shader module");

	GCMemory_Free(Shader->FragmentShaderData);
	GCMemory_Free(Shader->VertexShaderData);
}

void GCRendererShader_DestroyObjects(GCRendererShader* const Shader)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Shader->Device);

	vkDestroyShaderModule(DeviceHandle, Shader->FragmentShaderModuleHandle, NULL);
	vkDestroyShaderModule(DeviceHandle, Shader->VertexShaderModuleHandle, NULL);
}