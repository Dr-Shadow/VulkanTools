/*
 *
 * Copyright (C) 2015-2016 Valve Corporation
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
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tony Barbour <tony@LunarG.com>
 *
 */

#include "dev.h"
#include "mem.h"

VkResult intel_mem_alloc(struct intel_dev *dev,
                           const VkMemoryAllocateInfo *info,
                           struct intel_mem **mem_ret)
{
    struct intel_mem *mem;

    /* ignore any IMAGE_INFO and BUFFER_INFO usage: they don't alter allocations */

    mem = (struct intel_mem *) intel_base_create(&dev->base.handle,
            sizeof(*mem), dev->base.dbg, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, info, 0);
    if (!mem)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    mem->bo = intel_winsys_alloc_bo(dev->winsys,
            "vk-gpu-memory", info->allocationSize, 0);
    if (!mem->bo) {
        intel_mem_free(mem);
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    mem->size = info->allocationSize;

    *mem_ret = mem;

    return VK_SUCCESS;
}

void intel_mem_free(struct intel_mem *mem)
{
    intel_bo_unref(mem->bo);

    intel_base_destroy(&mem->base);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateMemory(
    VkDevice                                device,
    const VkMemoryAllocateInfo*                pAllocateInfo,
    const VkAllocationCallbacks*                     pAllocator,
    VkDeviceMemory*                         pMemory)
{
    struct intel_dev *dev = intel_dev(device);

    return intel_mem_alloc(dev, pAllocateInfo, (struct intel_mem **) pMemory);
}

VKAPI_ATTR void VKAPI_CALL vkFreeMemory(
    VkDevice                                  device,
    VkDeviceMemory                            mem_,
    const VkAllocationCallbacks*                     pAllocator)
{
    struct intel_mem *mem = intel_mem(mem_);

    intel_mem_free(mem);
}

VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory(
    VkDevice                                  device,
    VkDeviceMemory                            mem_,
    VkDeviceSize                              offset,
    VkDeviceSize                              size,
    VkFlags                                   flags,
    void**                                    ppData)
{
    struct intel_mem *mem = intel_mem(mem_);
    void *ptr = intel_mem_map(mem, flags);

    *ppData = (void *)((size_t)ptr + offset);

    return (ptr) ? VK_SUCCESS : VK_ERROR_MEMORY_MAP_FAILED;
}

VKAPI_ATTR void VKAPI_CALL vkUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              mem_)
{
    struct intel_mem *mem = intel_mem(mem_);

    intel_mem_unmap(mem);
}

VKAPI_ATTR VkResult VKAPI_CALL vkFlushMappedMemoryRanges(
    VkDevice                                  device,
    uint32_t                                  memoryRangeCount,
    const VkMappedMemoryRange*                pMemoryRanges)
{
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkInvalidateMappedMemoryRanges(
    VkDevice                                  device,
    uint32_t                                  memoryRangeCount,
    const VkMappedMemoryRange*                pMemoryRanges)
{
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceMemoryCommitment(
    VkDevice                                  device,
    VkDeviceMemory                            memory,
    VkDeviceSize*                             pCommittedMemoryInBytes)
{
}
