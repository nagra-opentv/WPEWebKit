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

#ifndef TextureMapperImageBuffer_h
#define TextureMapperImageBuffer_h

#include "BitmapTextureImageBuffer.h"
#include "ImageBuffer.h"
#include "TextureMapper.h"
#include "TextureMapperContextAttributes.h"
#include "GLContext.h"

#if USE(TEXTURE_MAPPER_CAIRO)
namespace WebCore {

class TextureMapperImageBuffer : public TextureMapper {
    WTF_MAKE_FAST_ALLOCATED;
public:
    TextureMapperImageBuffer();


    enum Flag {
        NoFlag = 0x00,
        ShouldBlend = 0x01,
        ShouldFlipTexture = 0x02,
        ShouldUseARBTextureRect = 0x04,
        ShouldAntialias = 0x08,
        ShouldRotateTexture90 = 0x10,
        ShouldRotateTexture180 = 0x20,
        ShouldRotateTexture270 = 0x40,
        ShouldConvertTextureBGRAToRGBA = 0x80,
        ShouldConvertTextureARGBToRGBA = 0x100
#if ENABLE(OPENTV_PLATFORM) && USE(OPENTV_PLAYER) && USE(OPENTV_PUNCH_HOLE_VIDEO) //Copyright 2017 OpenTV Inc. and Nagravision S.A.
        ,ShouldOverwriteRect = 0x200
#endif
    };

    typedef int Flags;

    // TextureMapper implementation
    virtual void drawBorder(const Color&, float borderWidth, const FloatRect&, const TransformationMatrix&) override;
//    virtual void drawRect(const Color&, float borderWidth, const FloatRect&, const TransformationMatrix&, GC3Denum, bool) override;
    virtual void drawNumber(int number, const Color& color, const FloatPoint& targetPoint, const TransformationMatrix& modelViewMatrix) override;
    virtual void drawTexture(const BitmapTexture&, const FloatRect& target, const TransformationMatrix& modelViewMatrix = TransformationMatrix(), float opacity = 1.0f, unsigned exposedEdges = AllEdges) override;

    virtual void drawSolidColor(const FloatRect&, const TransformationMatrix&, const Color&) override;

    virtual void beginClip(const TransformationMatrix&, const FloatRect&) override;
    virtual void bindSurface(BitmapTexture* surface) override;
    virtual void endClip() override;
    virtual IntRect clipBounds() override { return currentContext()->clipBounds(); }
    virtual IntSize maxTextureSize() const;
    virtual Ref<BitmapTexture> createTexture() override { return BitmapTextureImageBuffer::create(); }
    virtual Ref<BitmapTexture> createTexture(int internalFormat) override { return BitmapTextureImageBuffer::create(); }

    virtual void clearColor(const Color&) override;

    inline GraphicsContext* currentContext()
    {
        return m_currentSurface ? static_cast<BitmapTextureImageBuffer*>(m_currentSurface.get())->graphicsContext() : GLContext::current()->platformContext();
    }

private:
    RefPtr<BitmapTexture> m_currentSurface;
    TextureMapperContextAttributes m_contextAttributes;
};

}
#endif // USE(TEXTURE_MAPPER)

#endif // TextureMapperImageBuffer_h
