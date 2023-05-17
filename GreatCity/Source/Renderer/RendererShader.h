#ifndef GC_RENDERER_RENDERER_SHADER_H
#define GC_RENDERER_RENDERER_SHADER_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererShader GCRendererShader;
	typedef struct GCRendererDevice GCRendererDevice;

	GCRendererShader* GCRendererShader_Create(const GCRendererDevice* const Device, const char* const VertexShaderPath, const char* const FragmentShaderPath);
	void GCRendererShader_Destroy(GCRendererShader* Shader);

#ifdef __cplusplus
}
#endif

#endif