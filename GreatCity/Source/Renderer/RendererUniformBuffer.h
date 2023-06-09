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

#ifndef GC_RENDERER_RENDERER_UNIFORM_BUFFER_H
#define GC_RENDERER_RENDERER_UNIFORM_BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererUniformBuffer GCRendererUniformBuffer;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererCommandList GCRendererCommandList;

    typedef struct GCRendererUniformBufferDescription
    {
        const GCRendererDevice* Device;
        const GCRendererCommandList* CommandList;

        size_t DataSize;
    } GCRendererUniformBufferDescription;

	GCRendererUniformBuffer* GCRendererUniformBuffer_Create(const GCRendererUniformBufferDescription* const Description);
	void GCRendererUniformBuffer_Destroy(GCRendererUniformBuffer* UniformBuffer);

#ifdef __cplusplus
}
#endif

#endif