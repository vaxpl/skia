/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkFramebuffer.h"

#include "src/gpu/vk/GrVkAttachment.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkRenderPass.h"

GrVkFramebuffer* GrVkFramebuffer::Create(
        GrVkGpu* gpu,
        int width, int height,
        const GrVkRenderPass* renderPass,
        GrVkAttachment* colorAttachment,
        GrVkAttachment* resolveAttachment,
        GrVkAttachment* stencilAttachment,
        GrVkResourceProvider::CompatibleRPHandle compatibleRenderPassHandle) {
    // At the very least we need a renderPass and a colorAttachment
    SkASSERT(renderPass);
    SkASSERT(colorAttachment);

    VkImageView attachments[3];
    attachments[0] = colorAttachment->framebufferView()->imageView();
    int numAttachments = 1;
    if (resolveAttachment) {
        attachments[numAttachments++] = resolveAttachment->framebufferView()->imageView();
    }
    if (stencilAttachment) {
        attachments[numAttachments++] = stencilAttachment->framebufferView()->imageView();
    }

    VkFramebufferCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkFramebufferCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.renderPass = renderPass->vkRenderPass();
    createInfo.attachmentCount = numAttachments;
    createInfo.pAttachments = attachments;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;

    VkFramebuffer framebuffer;
    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, CreateFramebuffer(gpu->device(), &createInfo, nullptr,
                                                  &framebuffer));
    if (err) {
        return nullptr;
    }

    return new GrVkFramebuffer(gpu, framebuffer, sk_ref_sp(colorAttachment),
                               sk_ref_sp(resolveAttachment), sk_ref_sp(stencilAttachment),
                               compatibleRenderPassHandle);
}

GrVkFramebuffer::GrVkFramebuffer(const GrVkGpu* gpu,
                                 VkFramebuffer framebuffer,
                                 sk_sp<GrVkAttachment> colorAttachment,
                                 sk_sp<GrVkAttachment> resolveAttachment,
                                 sk_sp<GrVkAttachment> stencilAttachment,
                                 GrVkResourceProvider::CompatibleRPHandle compatibleRPHandle)
        : GrVkManagedResource(gpu)
        , fFramebuffer(framebuffer)
        , fColorAttachment(std::move(colorAttachment))
        , fResolveAttachment(std::move(resolveAttachment))
        , fStencilAttachment(std::move(stencilAttachment))
        , fCompatibleRenderPassHandle(compatibleRPHandle) {}

GrVkFramebuffer::GrVkFramebuffer(const GrVkGpu* gpu,
                                 sk_sp<GrVkAttachment> colorAttachment,
                                 sk_sp<const GrVkRenderPass> renderPass,
                                 std::unique_ptr<GrVkSecondaryCommandBuffer> externalCommandBuffer)
        : GrVkManagedResource(gpu)
        , fColorAttachment(std::move(colorAttachment))
        , fExternalRenderPass(std::move(renderPass))
        , fExternalCommandBuffer(std::move(externalCommandBuffer)) {}

GrVkFramebuffer::~GrVkFramebuffer() {}

void GrVkFramebuffer::freeGPUData() const {
    SkASSERT(this->isExternal() || fFramebuffer != VK_NULL_HANDLE);
    if (!this->isExternal()) {
        GR_VK_CALL(fGpu->vkInterface(), DestroyFramebuffer(fGpu->device(), fFramebuffer, nullptr));
    }

    // TODO: having freeGPUData virtual on GrManagedResource be const seems like a bad restriction
    // since we are changing the internal objects of these classes when it is called. We should go
    // back a revisit how much of a headache it would be to make this function non-const
    GrVkFramebuffer* nonConstThis = const_cast<GrVkFramebuffer*>(this);
    nonConstThis->releaseResources();
}

void GrVkFramebuffer::releaseResources() {
    if (fExternalCommandBuffer) {
        fExternalCommandBuffer->releaseResources();
        fExternalCommandBuffer.reset();
    }
}

void GrVkFramebuffer::returnExternalGrSecondaryCommandBuffer(
        std::unique_ptr<GrVkSecondaryCommandBuffer> cmdBuffer) {
    SkASSERT(!fExternalCommandBuffer);
    fExternalCommandBuffer = std::move(cmdBuffer);
}

std::unique_ptr<GrVkSecondaryCommandBuffer> GrVkFramebuffer::externalCommandBuffer() {
    SkASSERT(fExternalCommandBuffer);
    return std::move(fExternalCommandBuffer);
}
