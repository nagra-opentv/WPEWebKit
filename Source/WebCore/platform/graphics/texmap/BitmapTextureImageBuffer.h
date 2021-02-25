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

#ifndef BitmapTextureImageBuffer_h
#define BitmapTextureImageBuffer_h

#include "BitmapTexture.h"
#include "ImageBuffer.h"
#include "TextureMapperContextAttributes.h"
#include "IntRect.h"
#include "IntSize.h"

namespace WebCore {

class GraphicsContext;

typedef unsigned int TextureID;
typedef unsigned int TextureFormat;

class BitmapTextureImageBuffer : public BitmapTexture {
public:
    static Ref<BitmapTexture> create() { return adoptRef(*new BitmapTextureImageBuffer); }
    virtual IntSize size() const { return m_image->internalSize(); }
    virtual void didReset();
    virtual bool isValid() const { return m_image.get(); }
    inline GraphicsContext* graphicsContext() { return m_image ? &m_image->context() : nullptr; }
    virtual void updateContents(Image*, const IntRect&, const IntPoint&) override;
    virtual void updateContents(TextureMapper&, GraphicsLayer*, const IntRect& target, const IntPoint& offset, float) override;
    virtual void updateContents(const void*, const IntRect& target, const IntPoint& sourceOffset, int bytesPerLine) override;
       
    RefPtr<BitmapTexture> applyFilters(TextureMapper*, const FilterOperations&);
    ImageBuffer* image() const { return m_image.get(); }

    void copyFromExternalTexture(void* textureID){ assert(0);}
    virtual uint32_t id() const { return m_id; }

private:
    BitmapTextureImageBuffer();
    std::unique_ptr<ImageBuffer> m_image;
    uint32_t m_id;
};

}

#endif // BitmapTextureImageBuffer_h
