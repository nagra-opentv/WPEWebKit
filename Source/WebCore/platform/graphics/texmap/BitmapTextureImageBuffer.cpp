/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2014 Igalia S.L.
 * Copyright (C) 2018-2020 OpenTV, Inc. and Nagravision S.A. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"

#include "RefPtrCairo.h"
#include <cairo.h>

#include "BitmapTextureImageBuffer.h"
#if USE(OPENTV_PLAYER)
#include "MediaPlayerPrivateOpentvPlayer.h"
#endif
#include "GraphicsLayer.h"
#include "CairoOperations.h"

//#define OTV_ENABLE_CAIRO_AC_DEBUG 1
#ifdef OTV_ENABLE_CAIRO_AC_DEBUG
#define CAIRO_AC_DEBUG(msg, arg...) WTFLogAlways("Otvwebkit [CAIRO_AC] "#msg, ##arg)

#else
#define CAIRO_AC_DEBUG(msg, arg...) (void (0))
#endif


namespace WebCore {

BitmapTextureImageBuffer::BitmapTextureImageBuffer()
{
    CAIRO_AC_DEBUG("BitmapTextureImageBuffer::BitmapTextureImageBuffer\n");

}

void BitmapTextureImageBuffer::updateContents(const void* data, const IntRect& targetRect, const IntPoint& sourceOffset, int bytesPerLine)
{
    CAIRO_AC_DEBUG("BitmapTextureImageBuffer::updateContents data\n");

    RefPtr<cairo_surface_t> surface = adoptRef(cairo_image_surface_create_for_data(const_cast<unsigned char*>(static_cast<const unsigned char*>(data)),
        CAIRO_FORMAT_ARGB32, targetRect.width(), targetRect.height(), bytesPerLine));

#if 0
    m_image->context().platformContext()->drawSurfaceToContext(surface.get(), targetRect,
        IntRect(sourceOffset, targetRect.size()), m_image->context());
#else

    GraphicsContext & context = m_image->context();
    context.save();
    context.setCompositeOperation(CompositeCopy);

    Cairo::drawSurface(*m_image->context().platformContext(), surface.get(), targetRect, FloatRect(sourceOffset, targetRect.size()),
            InterpolationDefault, 1.0, Cairo::ShadowState());
    context.restore();
#endif

}

void BitmapTextureImageBuffer::updateContents(TextureMapper& textureMapper, GraphicsLayer* sourceLayer, const IntRect& targetRect, const IntPoint& sourceOffset, float scale)
{
    GraphicsContext & context = m_image->context();

    CAIRO_AC_DEBUG("BitmapTextureImageBuffer::updateContents TextureMapper\n");
    context.clearRect(targetRect);

    IntRect sourceRect(targetRect);
    sourceRect.setLocation(sourceOffset);
    context.save();
    context.clip(targetRect);
    context.translate(targetRect.x() - sourceOffset.x(), targetRect.y() - sourceOffset.y());
#if USE(OPENTV_PLAYER) && !USE(COORDINATED_GRAPHICS_THREADED)
    context.m_bitmapTexture = this;
#endif
    sourceLayer->paintGraphicsLayerContents(context, sourceRect);
#if USE(OPENTV_PLAYER) && !USE(COORDINATED_GRAPHICS_THREADED)
    for (const auto& player : context.m_players) 
        player->notifyMovePlayerRect(sourceOffset.x() - targetRect.x(), sourceOffset.y() - targetRect.y());

    for (const auto& player : m_players) 
    {
        IntRect videoRect = player->getPlayerRect();
        //videoRect.move(-sourceOffset.x() + targetRect.x(), -sourceOffset.y() + targetRect.y());

        if (!context.m_players.contains(player))
        {
            if (sourceRect.contains(videoRect))
            {
                player->notifyBitmapDestroyed(this);
            }
            else
                context.m_players.append(player);
        }
    }

    m_players.remove(0, m_players.size());
    m_players.swap(context.m_players);
#endif
    context.restore();
}

void BitmapTextureImageBuffer::didReset()
{
    if(m_image.get()){
        GraphicsContext & context = m_image->context();
        context.clearRect(FloatRect(IntRect(IntPoint(), m_image->logicalSize())));
    }
    else{
        m_image = ImageBuffer::create(contentSize(), Accelerated);
    }
}

void BitmapTextureImageBuffer::updateContents(Image* image, const IntRect& targetRect, const IntPoint& offset)
{
    CAIRO_AC_DEBUG("BitmapTextureImageBuffer::updateContents image(x=%f, y=%f), target(x=%d, y=%d, w=%d, h=%d), offset(%d, %d)\n", image->width(), image->height(), 
        targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height(), offset.x(), offset.y());
    GraphicsContext & context = m_image->context();
    //FloatRect target(targetRect);
    //FloatRect source(IntRect(offset, targetRect.size()));
    context.setCompositeOperation(CompositeCopy);
    context.drawImage(*image, targetRect, IntRect(offset, targetRect.size()));
}

} // namespace WebCore
