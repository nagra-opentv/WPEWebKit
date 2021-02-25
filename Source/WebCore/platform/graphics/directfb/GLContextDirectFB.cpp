/*
 * Copyright (C) 2012 Igalia, S.L.
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
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#if USE(TEXTURE_MAPPER_CAIRO)
#include <wtf/Forward.h>
#include "GLContextDirectFB.h"
#include "Performance.h"
#include "GraphicsContext.h"
#include <GraphicsContextImplCairo.h>
#include <cairo.h>
#if ENABLE(DIRECTFB)
#include "directfb.h"
#endif
//#define OTV_ENABLE_CAIRO_AC_DEBUG 1
#ifdef OTV_ENABLE_CAIRO_AC_DEBUG
#define CAIRO_AC_DEBUG(msg, arg...) WTFLogDebug("Otvwebkit [CAIRO_AC] "#msg, ##arg)
#else
#define CAIRO_AC_DEBUG(msg, arg...) (void (0))
#endif

namespace WebCore {

/* macro for a safe call to DirectFB functions */
//#define LOG_DIRECTFB
#if defined(LOG_DIRECTFB)
#define DFBCHECK( x )                      \
{                                          \
    DFBResult retval = DFB_OK;             \
    {                                      \
        printf( "about to %s... ", #x );   \
        printf( "\n" );                    \
    }                                      \
                                           \
    retval = ( x );                        \
    {                                      \
        printf( "%s returned: %s\n", #x,   \
        DirectFBErrorString( retval ) );   \
    }                                      \
}
#else
#define DFBCHECK(x)                         \
{                                           \
    DFBResult _dfbres = DFB_OK;             \
    _dfbres = ( x );                        \
    if (_dfbres != DFB_OK)                  \
        ASSERT(0); \
}
#endif

static IDirectFB            *s_dfb = 0;

static void unregisterDirectFBDisplayPlatform()
{
    if (s_dfb)
        s_dfb->Release(s_dfb);
}

std::unique_ptr<GLContextDirectFB> GLContextDirectFB::createWindowContext(GLNativeWindowType dfbwindow_id, GLContext* sharingContext,PlatformDisplay& display)
{
    UNUSED_PARAM(sharingContext);

    IDirectFBDisplayLayer *layer = NULL;
    IDirectFBSurface *dfbSurface = NULL;
    IDirectFBWindow *dfbWindow = NULL;

    if (!s_dfb)
        DirectFBCreate(&s_dfb);

    atexit(&unregisterDirectFBDisplayPlatform);

    CAIRO_AC_DEBUG("LayerTreeHostGtk::createWindowContext() s_dfb: %p\n", s_dfb);
    DFBCHECK(s_dfb->GetDisplayLayer(s_dfb, DLID_PRIMARY, &layer));
    CAIRO_AC_DEBUG("LayerTreeHostGtk::createWindowContext() layer: %p\n", layer);
    DFBCHECK(layer->SetCooperativeLevel(layer, DLSCL_SHARED));
    
#if 0
    DFBCHECK(layer->GetWindow(layer, static_cast<unsigned long>(dfbwindow_id), &dfbWindow));
    
    CAIRO_AC_DEBUG("LayerTreeHostGtk::createWindowContext() dfbwindow_id=%lu, dfbWindow: %p\n", static_cast<unsigned long>(dfbwindow_id), dfbWindow);
    
    DFBCHECK(dfbWindow->GetSurface(dfbWindow, &dfbSurface));
#else
    dfbSurface = reinterpret_cast<IDirectFBSurface*>(dfbwindow_id);
#endif
    CAIRO_AC_DEBUG("LayerTreeHostGtk::createWindowContext() dfbSurface: %p\n", dfbSurface);
    
    layer->Release(layer);


    RefPtr<cairo_surface_t> newSurface = adoptRef(cairo_directfb_surface_create(s_dfb, dfbSurface) );

    
    RefPtr<cairo_t> cr = adoptRef(cairo_create(newSurface.get()));

    if (!cr)
        return NULL;

    auto context = std::make_unique<GraphicsContext>(GraphicsContextImplCairo::createFactory(cr.get()));

    return std::unique_ptr<GLContextDirectFB>(new GLContextDirectFB(display,dfbSurface, dfbWindow, WTFMove(context), newSurface));
}


std::unique_ptr<GLContextDirectFB> GLContextDirectFB::createContext(GLNativeWindowType windowHandle, PlatformDisplay& display,GLContext* sharingContext)
{
    return createWindowContext(windowHandle, sharingContext,display);
}

GLContextDirectFB::GLContextDirectFB(PlatformDisplay& display,IDirectFBSurface *dfbSurface, IDirectFBWindow *window,  std::unique_ptr<GraphicsContext> context, RefPtr<cairo_surface_t> cairoSurf)
    : GLContext(display)
    , m_dfbSurface(dfbSurface)
    , m_window(window)
    , m_context(WTFMove(context))
    , m_cairoSurf(cairoSurf)
{
}

GLContextDirectFB::~GLContextDirectFB()
{
    CAIRO_AC_DEBUG("GLContextDirectFB::~GLContextDirectFB:");

    if (m_dfbSurface)
        m_dfbSurface->Release( m_dfbSurface );
    if (m_window)
        m_window->Release(m_window);
}

void GLContextDirectFB::swapBuffers()
{

    m_dfbSurface->Unlock(m_dfbSurface);
    DFBCHECK(m_dfbSurface->Flip( m_dfbSurface, NULL, DSFLIP_WAITFORSYNC ));
    CAIRO_AC_DEBUG("GLContextDirectFB::swapBuffers done");

}

void GLContextDirectFB::clearRect(const FloatRect& rect) 
{ 
    m_context->clearRect(rect); 
}

} // namespace WebCore

#endif // USE(TEXTURE_MAPPER_CAIRO)
