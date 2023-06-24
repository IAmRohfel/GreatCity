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

#ifndef GC_RENDERER_RENDERER_ENUMS_H
#define GC_RENDERER_RENDERER_ENUMS_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum GCRendererAttachmentType
    {
        GCRendererAttachmentType_Color,
        GCRendererAttachmentType_DepthStencil
    } GCRendererAttachmentType;

    typedef enum GCRendererFormat
    {
        GCRendererFormat_SRGB,
        GCRendererFormat_RGBA,
        GCRendererFormat_UnsignedInteger,
        GCRendererFormat_Byte,
        GCRendererFormat_D32
    } GCRendererFormat;

    typedef enum GCRendererSampleCount
    {
        GCRendererSampleCount_1,
        GCRendererSampleCount_2,
        GCRendererSampleCount_4,
        GCRendererSampleCount_8,

        GCRendererSampleCount_MaximumUsable
    } GCRendererSampleCount;

#ifdef __cplusplus
}
#endif

#endif