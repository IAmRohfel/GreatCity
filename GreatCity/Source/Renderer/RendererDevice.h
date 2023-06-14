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

#ifndef GC_RENDERER_RENDERER_DEVICE_H
#define GC_RENDERER_RENDERER_DEVICE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererDevice GCRendererDevice;

    typedef struct GCRendererDeviceCapabilities
    {
        bool IsAnisotropySupported;
        float MaximumAnisotropy;
    } GCRendererDeviceCapabilities;

    GCRendererDevice *GCRendererDevice_Create(void);
    void GCRendererDevice_WaitIdle(const GCRendererDevice *const Device);
    GCRendererDeviceCapabilities GCRendererDevice_GetDeviceCapabilities(const GCRendererDevice *const Device);
    void GCRendererDevice_Destroy(GCRendererDevice *Device);

#ifdef __cplusplus
}
#endif

#endif