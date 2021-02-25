/*
 Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 Copyright (C) 2018-2020 OpenTV, Inc. and Nagravision S.A. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "TextureMapperImageBuffer.h"
#include "BitmapTexturePool.h"
#include "GraphicsLayer.h"
#include "NotImplemented.h"
#include "CairoUtilities.h"

#if USE(TEXTURE_MAPPER)
//#define OTV_ENABLE_CAIRO_AC_DEBUG
#ifdef OTV_ENABLE_CAIRO_AC_DEBUG
#define CAIRO_AC_DEBUG(msg, arg...) WTFLogAlways("Otvwebkit [CAIRO_AC] "#msg, ##arg)
#else
#define CAIRO_AC_DEBUG(msg, arg...) (void (0))
#endif

#define THREE_D_RENDERING 0

namespace WebCore {

static const int s_maximumAllowedImageBufferDimension = 4096;

TextureMapperImageBuffer::TextureMapperImageBuffer()
    : TextureMapper()
{
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::TextureMapperImageBuffer enter\n");
    m_texturePool = std::make_unique<BitmapTexturePool>(m_contextAttributes);
}

IntSize TextureMapperImageBuffer::maxTextureSize() const
{
    return IntSize(s_maximumAllowedImageBufferDimension, s_maximumAllowedImageBufferDimension);
}

void TextureMapperImageBuffer::beginClip(const TransformationMatrix& matrix, const FloatRect& rect)
{
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::beginClip(%p) enter, x=%f, y=%f, w=%f, h=%f, thread=%p\n", this, rect.x(), rect.y(), rect.width(), rect.height(), pthread_self());
    GraphicsContext* context = currentContext();
    if (!context)
        return;
#if THREE_D_RENDERING
    TransformationMatrix previousTransform = context->get3DTransform();
#else
    AffineTransform previousTransform = context->getCTM();
#endif
    context->save();

#if THREE_D_RENDERING
    context->concat3DTransform(matrix);
#else
    context->concatCTM(matrix.toAffineTransform());
#endif

    context->clip(rect);

#if THREE_D_RENDERING
    context->set3DTransform(previousTransform);
#else
    context->setCTM(previousTransform);
#endif
}


void TextureMapperImageBuffer::endClip()
{
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::endClip(%p)\n", this);

    GraphicsContext* context = currentContext();
    if (!context)
        return;

    context->restore();
}

void TextureMapperImageBuffer::drawTexture(const BitmapTexture& texture, const FloatRect& target, const TransformationMatrix& modelViewMatrix, float opacity, unsigned exposedEdges)
{
    GraphicsContext* context = currentContext();
    if (!context)
        return;

    const BitmapTextureImageBuffer& textureImageBuffer = static_cast<const BitmapTextureImageBuffer&>(texture);
    ImageBuffer* image = textureImageBuffer.image();
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::drawTexture(%p) enter, opacity:%f, logicalSize:(w=%d, h=%d) internalSize:(w=%d, h=%d), exposedEdges=%d\n", this, opacity, image->logicalSize().width(), image->logicalSize().height(),
        image->internalSize().width(), image->internalSize().height(), exposedEdges);

    context->save();
    context->setAlpha(opacity);
#if THREE_D_RENDERING
    context->concat3DTransform(modelViewMatrix);
#else
    context->concatCTM(modelViewMatrix.toAffineTransform());
#endif

    // Copyright 2017 OpenTV Inc. and Nagravision S.A.
    // Fix blending of masked graphics layers in otvwebkit when running in Cairo AC mode. Masked layers are used when HTML application is using "-webkit-mask-image" together 
    // with "translate3d" css.
    context->drawImageBuffer(*image, target, FloatRect(IntRect(IntPoint(), image->logicalSize())),
                             isInMaskMode() ? CompositeDestinationIn : CompositeSourceOver);

    context->restore();
}

void TextureMapperImageBuffer::clearColor(const Color& color)
{
    GraphicsContext* context = currentContext();
    if (!context)
        return;
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::clearColor context=%p");

    context->save();

    FloatRect contextSize = {0,0,1920, 1080};

    if (m_currentSurface)
    {
        IntSize surfaceSize = m_currentSurface->size();
        contextSize.setWidth(surfaceSize.width());
        contextSize.setHeight(surfaceSize.height());

        CAIRO_AC_DEBUG("TextureMapperImageBuffer::clearColor %dx%d\n", surfaceSize.width(), surfaceSize.height());
    }
    context->fillRect(contextSize, color);
    context->restore();
}

void TextureMapperImageBuffer::drawSolidColor(const FloatRect& rect, const TransformationMatrix& matrix, const Color& color)
{
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::drawSolidColor enter\n");
    GraphicsContext* context = currentContext();
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::drawSolidColor enter, context=%p\n", context);

    if (!context)
        return;

    context->save();

    context->setCompositeOperation(isInMaskMode() ? CompositeDestinationIn : CompositeSourceOver);
#if THREE_D_RENDERING
    context->concat3DTransform(matrix);
#else
    context->concatCTM(matrix.toAffineTransform());
#endif

    context->fillRect(rect, color);    
    context->restore();
}

void TextureMapperImageBuffer::drawBorder(const Color& color, float borderWidth, const FloatRect& rect, const TransformationMatrix& matrix)
{
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::drawBorder(%p) enter\n", this);
    GraphicsContext* context = currentContext();

    if (!context)
        return;

    context->save();
#if THREE_D_RENDERING
    context->concat3DTransform(matrix);
#else
    context->concatCTM(matrix.toAffineTransform());
#endif

    context->setStrokeStyle(SolidStroke);
    context->setStrokeColor(color);
    context->strokeRect(rect, borderWidth);

    context->restore();
}

#if 0
void TextureMapperImageBuffer::drawRect(const Color& color, float borderWidth, const FloatRect& rect, const TransformationMatrix& matrix, GC3Denum en, bool flag)
{
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::drawRect(%p) enter, x=%f, y=%f, w=%f, h=%f\n", this, rect.x(), rect.y(), rect.width(), rect.height());
    GraphicsContext* context = currentContext();
    if (!context)
        return;

    context->save();
#if THREE_D_RENDERING
    context->concat3DTransform(matrix);
#else
    context->concatCTM(matrix.toAffineTransform());
#endif
    context->clearRect(rect);
    context->restore();
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::drawRect exit\n");
}
#endif

void TextureMapperImageBuffer::drawNumber(int number, const Color& color, const FloatPoint& targetPoint, const TransformationMatrix& modelViewMatrix)
{
    CAIRO_AC_DEBUG("TextureMapperImageBuffer::drawNumber(%p) enter\n", this);
    GraphicsContext* context = currentContext();

    if (!context)
        return;

    context->save();
#if USE(CAIRO)
    int pointSize = 8;

    CString counterString = String::number(number).ascii();
    // cairo_text_extents() requires a cairo_t, so dimensions need to be guesstimated.
    int width = counterString.length() * pointSize * 1.2;
    int height = pointSize * 1.5;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);

    // Since we won't swap R+B when uploading a texture, paint with the swapped R+B color.
    if (color.isExtended())
        cairo_set_source_rgba(cr, color.asExtended().blue(), color.asExtended().green(), color.asExtended().red(), color.asExtended().alpha());
    else {
        float r, g, b, a;
        color.getRGBA(r, g, b, a);
        cairo_set_source_rgba(cr, b, g, r, a);
    }

    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    cairo_select_font_face(cr, "Monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, pointSize);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 2, pointSize);
    cairo_show_text(cr, counterString.data());

    IntSize size(width, height);
    IntRect sourceRect(IntPoint::zero(), size);
    IntRect targetRect(roundedIntPoint(targetPoint), size);

    RefPtr<BitmapTexture> texture = acquireTextureFromPool(size);
    const unsigned char* bits = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    static_cast<BitmapTextureImageBuffer*>(texture.get())->updateContents(bits, sourceRect, IntPoint::zero(), stride);
    drawTexture(*texture, targetRect, modelViewMatrix, 1.0f, AllEdges);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);
#endif

    context->restore();
}

void TextureMapperImageBuffer::bindSurface(BitmapTexture* surface)
{
    if(surface)
       CAIRO_AC_DEBUG("TextureMapperImageBuffer::bindSurface(%p) %dx%d", this, surface->size().width(), surface->size().height());
    else
       CAIRO_AC_DEBUG("TextureMapperImageBuffer::bindSurface(%p) surface is %p", this,surface);

    m_currentSurface = surface;
}

std::unique_ptr<TextureMapper> TextureMapper::platformCreateAccelerated()
{
    return std::make_unique<TextureMapperImageBuffer>();
}

}
#endif
