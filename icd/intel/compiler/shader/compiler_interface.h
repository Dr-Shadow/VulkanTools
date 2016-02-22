/*
 *
 * Copyright (C) 2015-2016 Valve Corporation
 * Copyright (C) 2015-2016 LunarG, Inc.
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
 * Author: Cody Northrop <cody@lunarg.com>
 *
 */

#ifndef COMPILER_INTERFACE_H
#define COMPILER_INTERFACE_H

#include <icd.h>

#ifdef __cplusplus
extern "C" {
#endif

struct gl_context;

void initialize_mesa_context_to_defaults(struct gl_context *ctx);

struct intel_ir *shader_create_ir(const struct intel_gpu *gpu,
                                  const void *code, size_t size,
                                  VkShaderStageFlagBits stage);

void shader_create_ir_with_lock(const struct intel_gpu *gpu,
                                const void *code, size_t size,
                                VkShaderStageFlagBits stage,
                                struct intel_ir **ir);

void shader_destroy_ir(struct intel_ir *ir);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* COMPILER_INTERFACE_H */
