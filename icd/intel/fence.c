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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 *
 */

#include "kmd/winsys.h"
#include "cmd.h"
#include "dev.h"
#include "wsi.h"
#include "fence.h"
#include "instance.h"

static void fence_destroy(struct intel_obj *obj)
{
    struct intel_fence *fence = intel_fence_from_obj(obj);

    intel_fence_destroy(fence);
}

VkResult intel_fence_create(struct intel_dev *dev,
                              const VkFenceCreateInfo *info,
                              struct intel_fence **fence_ret)
{
    struct intel_fence *fence;

    fence = (struct intel_fence *) intel_base_create(&dev->base.handle,
            sizeof(*fence), dev->base.dbg, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, info, 0);
    if (!fence)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    if (dev->base.handle.instance->global_exts[INTEL_GLOBAL_EXT_WSI_SURFACE]) {
        VkResult ret = intel_wsi_fence_init(fence);
        if (ret != VK_SUCCESS) {
            intel_fence_destroy(fence);
            return ret;
        }
    }

    fence->obj.destroy = fence_destroy;

    *fence_ret = fence;
    fence->signaled = (info->flags & VK_FENCE_CREATE_SIGNALED_BIT);

    return VK_SUCCESS;
}

void intel_fence_destroy(struct intel_fence *fence)
{
    if (fence->wsi_data)
        intel_wsi_fence_cleanup(fence);

    intel_bo_unref(fence->seqno_bo);

    intel_base_destroy(&fence->obj.base);
}

void intel_fence_copy(struct intel_fence *fence,
                      const struct intel_fence *src)
{
    intel_wsi_fence_copy(fence, src);
    intel_fence_set_seqno(fence, src->seqno_bo);
}

void intel_fence_set_seqno(struct intel_fence *fence,
                           struct intel_bo *seqno_bo)
{
    intel_bo_unref(fence->seqno_bo);
    fence->seqno_bo = intel_bo_ref(seqno_bo);

    fence->signaled = false;
}

VkResult intel_fence_wait(struct intel_fence *fence, int64_t timeout_ns)
{
    VkResult ret;

    ret = intel_wsi_fence_wait(fence, timeout_ns);
    if (ret != VK_SUCCESS)
        return ret;

    if (fence->signaled) {
        return VK_SUCCESS;
    }

    if (fence->seqno_bo) {
        ret = (intel_bo_wait(fence->seqno_bo, timeout_ns)) ?
            VK_NOT_READY : VK_SUCCESS;
        if (ret == VK_SUCCESS) {
            fence->signaled = true;
        }
        return ret;
    }

    assert(0 && "Invalid fence status");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFence(
    VkDevice                                  device,
    const VkFenceCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                     pAllocator,
    VkFence*                                  pFence)
{
    struct intel_dev *dev = intel_dev(device);

    return intel_fence_create(dev, pCreateInfo,
            (struct intel_fence **) pFence);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFence(
    VkDevice                                device,
    VkFence                                 fence,
    const VkAllocationCallbacks*                     pAllocator)

 {
    struct intel_obj *obj = intel_obj(fence);

    obj->destroy(obj);
 }

VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceStatus(
    VkDevice                                  device,
    VkFence                                   fence_)
{
    struct intel_fence *fence = intel_fence(fence_);

    return intel_fence_wait(fence, 0);
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitForFences(
    VkDevice                                  device,
    uint32_t                                    fenceCount,
    const VkFence*                            pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout)
{
    VkResult ret = VK_SUCCESS;
    uint32_t i;

    for (i = 0; i < fenceCount; i++) {
        struct intel_fence *fence = intel_fence(pFences[i]);
        int64_t ns;
        VkResult r;

        /* timeout in nano seconds */
        ns = (timeout <= (uint64_t) INT64_MAX) ? timeout : -1;
        r = intel_fence_wait(fence, ns);

        if (!waitAll && r == VK_SUCCESS)
            return VK_SUCCESS;

        /* Translate return value according to spec */
        if (r == VK_NOT_READY)
            r = VK_TIMEOUT;

        if (r != VK_SUCCESS)
            ret = r;
    }

    return ret;
}
VKAPI_ATTR VkResult VKAPI_CALL vkResetFences(
    VkDevice                                  device,
    uint32_t                                  fenceCount,
    const VkFence*                            pFences)
{
    uint32_t i;

    for (i = 0; i < fenceCount; i++) {
        struct intel_fence *fence = intel_fence(pFences[i]);
        fence->signaled = false;
    }

    return VK_SUCCESS;
}
