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

#ifndef GC_RENDERER_RENDERER_SHADER_H
#define GC_RENDERER_RENDERER_SHADER_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererShader GCRendererShader;
    typedef struct GCRendererDevice GCRendererDevice;

    typedef struct GCRendererShaderDescription
    {
        const GCRendererDevice* Device;
        const char* VertexShaderPath;
        const char* FragmentShaderPath;
    } GCRendererShaderDescription;

    GCRendererShader* GCRendererShader_Create(const GCRendererShaderDescription* const Description);
    void GCRendererShader_Destroy(GCRendererShader* Shader);

#ifdef __cplusplus
}
#endif

#endif