/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkColorData.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrTextureEffect.h"
#endif
#include "src/core/SkClipOpPriv.h"

namespace {

class SkBlendImageFilter : public SkImageFilter_Base {
public:
    SkBlendImageFilter(SkBlendMode mode, sk_sp<SkImageFilter> inputs[2],
                       const SkRect* cropRect)
          : INHERITED(inputs, 2, cropRect)
          , fMode(mode) {}

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> filterImageGPU(const Context& ctx,
                                         sk_sp<SkSpecialImage> background,
                                         const SkIPoint& backgroundOffset,
                                         sk_sp<SkSpecialImage> foreground,
                                         const SkIPoint& foregroundOffset,
                                         const SkIRect& bounds) const;
#endif

    void flatten(SkWriteBuffer&) const override;

    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;

private:
    friend void ::SkRegisterBlendImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlendImageFilter)

    SkBlendMode fMode;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Blend(SkBlendMode mode,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    return sk_sp<SkImageFilter>(new SkBlendImageFilter(mode, inputs, cropRect));
}

void SkRegisterBlendImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlendImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkXfermodeImageFilter_Base", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("SkXfermodeImageFilterImpl", SkBlendImageFilter::CreateProc);
}

static unsigned unflatten_blendmode(SkReadBuffer& buffer) {
    unsigned mode = buffer.read32();
    (void)buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode);
    return mode;
}

sk_sp<SkFlattenable> SkBlendImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    unsigned mode = unflatten_blendmode(buffer);
    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkImageFilters::Blend((SkBlendMode)mode, common.getInput(0), common.getInput(1),
                                 common.cropRect());
}

void SkBlendImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.write32((unsigned)fMode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkBlendImageFilter::onFilterImage(const Context& ctx,
                                                        SkIPoint* offset) const {
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> background(this->filterInput(0, ctx, &backgroundOffset));

    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> foreground(this->filterInput(1, ctx, &foregroundOffset));

    SkIRect foregroundBounds = SkIRect::MakeEmpty();
    if (foreground) {
        foregroundBounds = SkIRect::MakeXYWH(foregroundOffset.x(), foregroundOffset.y(),
                                             foreground->width(), foreground->height());
    }

    SkIRect srcBounds = SkIRect::MakeEmpty();
    if (background) {
        srcBounds = SkIRect::MakeXYWH(backgroundOffset.x(), backgroundOffset.y(),
                                      background->width(), background->height());
    }

    srcBounds.join(foregroundBounds);
    if (srcBounds.isEmpty()) {
        return nullptr;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();

#if SK_SUPPORT_GPU
    if (ctx.gpuBacked()) {
        return this->filterImageGPU(ctx, background, backgroundOffset,
                                    foreground, foregroundOffset, bounds);
    }
#endif

    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0); // can't count on background to fully clear the background
    canvas->translate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));

    if (background) {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        background->draw(canvas,
                         SkIntToScalar(backgroundOffset.fX), SkIntToScalar(backgroundOffset.fY),
                         SkSamplingOptions(), &paint);
    }

    this->drawForeground(canvas, foreground.get(), foregroundBounds);

    return surf->makeImageSnapshot();
}

SkIRect SkBlendImageFilter::onFilterBounds(const SkIRect& src,
                                           const SkMatrix& ctm,
                                           MapDirection dir,
                                           const SkIRect* inputRect) const {
    if (kReverse_MapDirection == dir) {
        return INHERITED::onFilterBounds(src, ctm, dir, inputRect);
    }

    SkASSERT(!inputRect);
    SkASSERT(2 == this->countInputs());
    auto getBackground = [&]() {
        return this->getInput(0) ? this->getInput(0)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    auto getForeground = [&]() {
        return this->getInput(1) ? this->getInput(1)->filterBounds(src, ctm, dir, inputRect) : src;
    };
    switch (fMode) {
        case SkBlendMode::kClear:
            return SkIRect::MakeEmpty();

        case SkBlendMode::kSrc:
        case SkBlendMode::kDstATop:
            return getForeground();

        case SkBlendMode::kDst:
        case SkBlendMode::kSrcATop:
            return getBackground();

        case SkBlendMode::kSrcIn:
        case SkBlendMode::kDstIn: {
            auto result = getBackground();
            if (!result.intersect(getForeground())) {
                return SkIRect::MakeEmpty();
            }
            return result;
        }

        default: {
            auto result = getBackground();
            result.join(getForeground());
            return result;
        }
    }
}

void SkBlendImageFilter::drawForeground(SkCanvas* canvas, SkSpecialImage* img,
                                        const SkIRect& fgBounds) const {
    SkPaint paint;
    paint.setBlendMode(fMode);
    if (img) {
        img->draw(canvas, SkIntToScalar(fgBounds.fLeft), SkIntToScalar(fgBounds.fTop),
                  SkSamplingOptions(), &paint);
    }

    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect(SkRect::Make(fgBounds), kDifference_SkClipOp);
    paint.setColor(0);
    canvas->drawPaint(paint);
}

#if SK_SUPPORT_GPU

#include "src/gpu/effects/GrBlendFragmentProcessor.h"

sk_sp<SkSpecialImage> SkBlendImageFilter::filterImageGPU(const Context& ctx,
                                                         sk_sp<SkSpecialImage> background,
                                                         const SkIPoint& backgroundOffset,
                                                         sk_sp<SkSpecialImage> foreground,
                                                         const SkIPoint& foregroundOffset,
                                                         const SkIRect& bounds) const {
    SkASSERT(ctx.gpuBacked());

    auto context = ctx.getContext();

    GrSurfaceProxyView backgroundView, foregroundView;

    if (background) {
        backgroundView = background->view(context);
    }

    if (foreground) {
        foregroundView = foreground->view(context);
    }

    std::unique_ptr<GrFragmentProcessor> fp;
    const auto& caps = *ctx.getContext()->priv().caps();
    GrSamplerState sampler(GrSamplerState::WrapMode::kClampToBorder,
                           GrSamplerState::Filter::kNearest);

    if (backgroundView.asTextureProxy()) {
        SkRect bgSubset = SkRect::Make(background->subset());
        SkMatrix bgMatrix = SkMatrix::Translate(
                SkIntToScalar(bgSubset.left() - backgroundOffset.fX),
                SkIntToScalar(bgSubset.top()  - backgroundOffset.fY));
        fp = GrTextureEffect::MakeSubset(std::move(backgroundView), background->alphaType(),
                                         bgMatrix, sampler, bgSubset, caps);
        fp = GrColorSpaceXformEffect::Make(std::move(fp), background->getColorSpace(),
                                           background->alphaType(), ctx.colorSpace(),
                                           kPremul_SkAlphaType);
    } else {
        fp = GrFragmentProcessor::MakeColor(SK_PMColor4fTRANSPARENT);
    }

    if (foregroundView.asTextureProxy()) {
        SkRect fgSubset = SkRect::Make(foreground->subset());
        SkMatrix fgMatrix = SkMatrix::Translate(
                SkIntToScalar(fgSubset.left() - foregroundOffset.fX),
                SkIntToScalar(fgSubset.top()  - foregroundOffset.fY));
        auto fgFP = GrTextureEffect::MakeSubset(std::move(foregroundView), foreground->alphaType(),
                                                fgMatrix, sampler, fgSubset, caps);
        fgFP = GrColorSpaceXformEffect::Make(std::move(fgFP), foreground->getColorSpace(),
                                             foreground->alphaType(), ctx.colorSpace(),
                                             kPremul_SkAlphaType);
        fp = GrBlendFragmentProcessor::Make(std::move(fgFP), std::move(fp), fMode);
    }

    GrImageInfo info(ctx.grColorType(), kPremul_SkAlphaType, ctx.refColorSpace(), bounds.size());
    auto surfaceFillContext = GrSurfaceFillContext::Make(context, info, SkBackingFit::kApprox);
    if (!surfaceFillContext) {
        return nullptr;
    }

    surfaceFillContext->fillRectToRectWithFP(bounds,
                                             SkIRect::MakeSize(bounds.size()),
                                             std::move(fp));

    return SkSpecialImage::MakeDeferredFromGpu(context,
                                               SkIRect::MakeWH(bounds.width(), bounds.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               surfaceFillContext->readSurfaceView(),
                                               surfaceFillContext->colorInfo().colorType(),
                                               surfaceFillContext->colorInfo().refColorSpace());
}

#endif
