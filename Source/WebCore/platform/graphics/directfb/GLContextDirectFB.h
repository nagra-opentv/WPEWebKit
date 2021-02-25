/*
 * Copyright (C) 2012 Igalia S.L.
 * Copyright (C) 2018-2020 OpenTV, Inc. and Nagravision S.A. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301 USA
 */

#ifndef GLContextDirectFB_h
#define GLContextDirectFB_h

#if USE(TEXTURE_MAPPER_CAIRO)

#include "GLContext.h"
#include <cairo-directfb.h>

namespace WebCore {

class GraphicsContext;
class GLContextDirectFB final : public GLContext {
    WTF_MAKE_NONCOPYABLE(GLContextDirectFB);
public:
    static std::unique_ptr<GLContextDirectFB> createContext(GLNativeWindowType, PlatformDisplay& ,GLContext* sharingContext = 0);
    static std::unique_ptr<GLContextDirectFB> createWindowContext(GLNativeWindowType, GLContext* sharingContext,PlatformDisplay& );

    virtual ~GLContextDirectFB();
    virtual void swapBuffers();
    virtual void waitNative() {}
    virtual bool canRenderToDefaultFramebuffer() { return false; }
    virtual IntSize defaultFrameBufferSize() { return IntSize();}
    virtual cairo_device_t* cairoDevice() { return NULL;}
    virtual bool isEGLContext() const { return false; }
    virtual GraphicsContext* platformContext() override { return m_context.get(); };
    virtual void clearRect(const FloatRect& rect);
#if ENABLE(OPENTV_PLATFORM) && defined(OPENTV_FEATURE_OPENGL_INFO_API) //Copyright 2017 OpenTV Inc. and Nagravision S.A.
    virtual char const* getGLVersion() { return NULL;}
#endif
     void swapInterval(int) override {}
private:
    GLContextDirectFB(PlatformDisplay&,IDirectFBSurface *dfbSurface, IDirectFBWindow *window, std::unique_ptr<GraphicsContext> context, RefPtr<cairo_surface_t> cairoSurf);

    IDirectFBSurface *m_dfbSurface;
    IDirectFBWindow *m_window;
    std::unique_ptr<GraphicsContext> m_context;
    RefPtr<cairo_surface_t> m_cairoSurf;
    cairo_device_t* m_cairoDevice { nullptr };
};

} // namespace WebCore

#endif // USE(TEXTURE_MAPPER_CAIRO)

#endif // GLContextDirectFB_h
