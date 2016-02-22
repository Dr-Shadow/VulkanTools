/*
 *
 * Copyright (C) 2015 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chia-I Wu <olv@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 *
 */

#ifndef ICD_FORMAT_H
#define ICD_FORMAT_H

#include <stdbool.h>
#include "icd.h"

static inline bool icd_format_is_undef(VkFormat format)
{
    return (format == VK_FORMAT_UNDEFINED);
}

bool icd_format_is_ds(VkFormat format);

static inline bool icd_format_is_color(VkFormat format)
{
    return !(icd_format_is_undef(format) || icd_format_is_ds(format));
}

bool icd_format_is_norm(VkFormat format);

bool icd_format_is_int(VkFormat format);

bool icd_format_is_float(VkFormat format);

bool icd_format_is_srgb(VkFormat format);

bool icd_format_is_compressed(VkFormat format);

static inline int icd_format_get_block_width(VkFormat format)
{
    /* all compressed formats use 4x4 blocks */
    return (icd_format_is_compressed(format)) ? 4 : 1;
}

static inline bool icd_blend_mode_is_dual_src(VkBlendFactor mode)
{
    return (mode == VK_BLEND_FACTOR_SRC1_COLOR) ||
           (mode == VK_BLEND_FACTOR_SRC1_ALPHA) ||
           (mode == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR) ||
           (mode == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);
}

static inline bool icd_pipeline_cb_att_needs_dual_source_blending(const VkPipelineColorBlendAttachmentState *att)
{
    if (icd_blend_mode_is_dual_src(att->srcColorBlendFactor) ||
        icd_blend_mode_is_dual_src(att->srcAlphaBlendFactor) ||
        icd_blend_mode_is_dual_src(att->dstColorBlendFactor) ||
        icd_blend_mode_is_dual_src(att->dstAlphaBlendFactor)) {
        return true;
    }
    return false;
}

size_t icd_format_get_size(VkFormat format);

unsigned int icd_format_get_channel_count(VkFormat format);

void icd_format_get_raw_value(VkFormat format,
                              const uint32_t color[4],
                              void *value);

#endif /* ICD_FORMAT_H */
