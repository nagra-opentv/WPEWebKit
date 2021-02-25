/*
 * Copyright (C) 2020 OpenTV, Inc. and Nagravision S.A. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include "config.h"

#if ENABLE(ACCELERATED_PAINTING)
#include "NicosiaDirectfbBuffer.h"
#include <wtf/RunLoop.h>
#include "RefPtrCairo.h"
#include <cairo.h>
#include <utility>
#include "CairoUtilities.h"

namespace Nicosia {

NicosiaDirectfbBuffer::NicosiaDirectfbBuffer(const WebCore::IntSize& size, Flags flags):
    Buffer(size, flags),
    m_pRawBuffer(NULL),
    m_pDFBSurface(NULL)
{
    if (flags & Accelerated)
    {
        static bool logged_once = false;
        if(!logged_once){
            WTFLogAlways("NicosiaDirectfbBuffer: using hardware accelerated buffers");
            logged_once = true;
        }
        m_pDFBSurface = createDFBSurface(DSPF_ARGB,
                                        this->size(),
                                        NULL);
    }
}

NicosiaDirectfbBuffer::~NicosiaDirectfbBuffer()
{
    unlock();
    if(m_pDFBSurface){
        WebCore::releaseDFBSurface(m_pDFBSurface);
    }
}

void NicosiaDirectfbBuffer::releaseSurface()
{
    // lock DFB surface and obtain the pointer to data. It will be used later in data() function call
    m_pRawBuffer = lock();
}

IDirectFBSurface* NicosiaDirectfbBuffer::dfbSurface()
{
    ASSERT(m_pDFBSurface);
    return m_pDFBSurface;
}

unsigned char* NicosiaDirectfbBuffer::data() const
{
    if(m_pRawBuffer)
    {        
        return m_pRawBuffer;
    }
    else
        return Buffer::data();
}

unsigned char* NicosiaDirectfbBuffer::lock() {
    if(m_pDFBSurface)
    {
        MonotonicTime startTime = MonotonicTime::now();
        unsigned char* rawDataPtr = NULL;
        int pitch = 0;
        m_pDFBSurface->Lock(m_pDFBSurface, DFBSurfaceLockFlags(DSLF_READ), (void**)&rawDataPtr, &pitch);

#if 0
        static double total = 0.0;
        static unsigned int num_calls = 0;
        double diff_ms = (MonotonicTime::now() - startTime).milliseconds();

        total += diff_ms;
        num_calls++;

        WTFLogAlways("NicosiaDirectfbBuffer::lock(Lock) took %f ms, ave = %f", diff_ms, total/(double)num_calls);
#endif 
        return rawDataPtr;
    }
}

void NicosiaDirectfbBuffer::unlock()
{  
    if(m_pDFBSurface)
    {
        m_pDFBSurface->Unlock(m_pDFBSurface);
    }
}

} // namespace Nicosia

#endif // #if ENABLE(ACCELERATED_PAINTING)
