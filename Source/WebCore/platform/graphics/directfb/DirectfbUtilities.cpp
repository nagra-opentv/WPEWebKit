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

#include "config.h"

#if ENABLE(DIRECTFB)

#include "CairoUtilities.h"
#include "FloatPoint.h"
#include "FloatRect.h"
#include "IntRect.h"
#include <wtf/Assertions.h>
#include <wtf/MonotonicTime.h>
#include "DataCache.h"
#include <wtf/Lock.h>

namespace WebCore {

static int numReleaased = 0;
static int numAllocated = 0;

#define USE_DFB_SURFACE_CACHE 1

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x)                         \
{                                           \
    static DFBResult _dfbres = DFB_OK;      \
    _dfbres = ( x );                        \
    if (_dfbres != DFB_OK)                  \
        printf( "DirectFB Error: %s returned %s\n", #x, DirectFBErrorString( _dfbres ) ); \
}
#if USE_DFB_SURFACE_CACHE
static DataCache *surfaceCache = NULL;
static Lock surfaceCacheLock;
static const unsigned int surfaceCacheMaxBytes = 50*1024*1024; // 50 MB
static const unsigned int surfaceCacheMaxSurfaces = 30;
static unsigned int surfaceCacheCurBytes = 0;
static const unsigned int bytesPerPixel = 4;

typedef struct _DfbSurfaceKey
{
    int width;
    int height;
} DfbSurfaceKey_t;
#endif

#if USE_DFB_SURFACE_CACHE
/** Comparison function to sort/find surfaces.
  * @param[in] keyA First key value to compare
  * @param[in] keyB Second key value to compare
  *
  * @return  a negative integer if the first key value comes before the second, 
  *    0 if they are equal, or a positive integer if the first value comes after the second.
  */
gint surfaceCompareFunc (gconstpointer keyA, gconstpointer keyB)
{
    DfbSurfaceKey_t *surfA = (DfbSurfaceKey_t*)keyA;
    DfbSurfaceKey_t *surfB = (DfbSurfaceKey_t*)keyB;
    gint res = -1;
    if(surfA->width > surfB->width)
    {
        res = 1;
    }
    else if(surfA->width == surfB->width)
    {
        if(surfA->height == surfB->height)
        {
            res = 0;
        }
        else if(surfA->height > surfB->height)
        {
            res = 1;
        }
    }
    
    return res;
}

/** Delete function implemented by caller to delete data previously inserted into cache.
  * @param[in] data  Data to be deleted from cache.
  *
  * @return None. 
  */
void surfaceDeleteFunc (gpointer data)
{
    IDirectFBSurface* surf_p = (IDirectFBSurface*)data;

 //   UNATTACHED_MEM_DECREMENT( surf_p )

    int width = 0, height = 0;
    surf_p->GetSize(surf_p, &width, &height);
    surfaceCacheCurBytes -= (width*height*bytesPerPixel);
//    WTFLogAlways("Deleting surface (%dx%d) bytes = %d, cacheCurBytes=%d, cache size = %d", width, height, (width*height*4), surfaceCacheCurBytes, surfaceCache->size());

    numReleaased++;
    surf_p->Release(surf_p);
}

/** Delete function implemented by caller to delete cache key.
  * @param[in] key  key to be deleted from cache.
  *
  * @return None. 
  */
void surfaceKeyDeleteFunc (gpointer key)
{
    DfbSurfaceKey_t *surfKey = (DfbSurfaceKey_t *)key;
    delete surfKey;
}

/** Delete function implemented by caller to dump cache data.
  * @param[in] key  key to be dumped.
  * @param[in] data  data to be dumped.
  *
  * @return None. 
  */
void dumpCacheData(gpointer key, gpointer data)
{
    DfbSurfaceKey_t *key_p = (DfbSurfaceKey_t *) key;
//    WTFLogAlways(" %dx%d ",key_p->width, key_p->height);
}
#endif /* end of USE_DFB_SURFACE_CACHE*/

IDirectFB* dfb()
{
    static IDirectFB* s_dfb = NULL;
    if(!s_dfb)
    {
        DirectFBInit(NULL, NULL);
        DFBCHECK( DirectFBCreate(&s_dfb) );

#if USE_DFB_SURFACE_CACHE
        WTFLogAlways("DirectFB Surface Cache: surfaceCacheMaxSurfaces=%d, surfaceCacheMaxBytes=%d\n", surfaceCacheMaxSurfaces, surfaceCacheMaxBytes);
        surfaceCache = new DataCache(surfaceCacheMaxSurfaces, surfaceCompareFunc, surfaceKeyDeleteFunc, surfaceDeleteFunc);
        surfaceCache->setDumpCacheDataFunc(dumpCacheData);
#else
        WTFLogAlways("DirectFB Surface Cache disabled\n");
#endif
    }
    return s_dfb;
}


void releaseDFBSurface(IDirectFBSurface* pSurface)
{
#if USE_DFB_SURFACE_CACHE
    LockHolder locker(surfaceCacheLock);


    if(surfaceCacheCurBytes >= surfaceCacheMaxBytes){
        WTFLogAlways("Reached surfaceCache capacity (surfaceCacheCurBytes=%d, surfaceCacheMaxBytes=%d, cache size = %d)\n", surfaceCacheCurBytes, surfaceCacheMaxBytes, surfaceCache->size());

        pSurface->Release(pSurface);
        numReleaased++;
        return;
    }

    DfbSurfaceKey_t *surfKey = new _DfbSurfaceKey;
    
    int width = 0, height = 0;
    pSurface->GetSize(pSurface, &width, &height);
    surfKey->width = width;
    surfKey->height = height;
    int res = surfaceCache->insertDataIntoCache(surfKey, (gpointer) pSurface);
    if(res != 0)
    {
        WTFLogAlways("Error in inserting directfb surface into cache\n");
        pSurface->Release(pSurface);
        numReleaased++;
    }
    else
    {
        surfaceCacheCurBytes += (width*height*bytesPerPixel);
  //      WTFLogAlways("\n Inserted (%dx%d) bytes = %d, surfaceCacheCurBytes = %d, cache size = %d",surfKey->width, surfKey->height, (width*height*4), surfaceCacheCurBytes, surfaceCache->size());
        surfaceCache->dumpCacheData();
    }
#else // #if USE_DFB_SURFACE_CACHE
    pSurface->Release(pSurface);
    numReleaased++;
#endif
}

IDirectFBSurface* createDFBSurface(DFBSurfacePixelFormat format, const IntSize& size, unsigned *data )
{
    /*Create one instance of IDirectFB*/

    IDirectFB* s_dfb = dfb();

    IDirectFBSurface* dfbNativeSurface = NULL;

#if USE_DFB_SURFACE_CACHE
    {
        LockHolder locker(surfaceCacheLock);
        // WTFLogAlways("numAllocated - numReleased = %d, surfaceCacheCurBytes = %d", numAllocated - numReleaased, surfaceCacheCurBytes);

        DfbSurfaceKey_t surfKey = {size.width(), size.height()};
        dfbNativeSurface = (IDirectFBSurface*)surfaceCache->findCachedData((gpointer) &surfKey);
        if(dfbNativeSurface){
            surfaceCacheCurBytes -= (size.width()*size.height()*bytesPerPixel);
        }
    }
#endif 

    if (dfbNativeSurface == NULL)
    {
        DFBSurfaceDescription dsc;
        dsc.flags = DFBSurfaceDescriptionFlags(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_CAPS);
        dsc.width = size.width();
        dsc.height = size.height();
        dsc.pixelformat = format;
        dsc.caps = static_cast<DFBSurfaceCapabilities>(DSCAPS_STATIC_ALLOC | DSCAPS_PREMULTIPLIED | DSCAPS_VIDEOONLY);

        MonotonicTime startTime = MonotonicTime::now();

        DFBCHECK(s_dfb->CreateSurface(s_dfb, &dsc, &dfbNativeSurface));
        numAllocated++;

#if 0
        static double total = 0.0;
        static unsigned int num_calls = 0;
        double diff_ms = (MonotonicTime::now() - startTime).milliseconds();

        total += diff_ms;
        num_calls++;

        WTFLogAlways("[startTime: %f] s_dfb->CreateSurface took %f ms, ave = %f (gSwappingBuffers=%d, thread = %p)",
                    startTime.secondsSinceEpoch().seconds(),
                    diff_ms,
                    total/(double)num_calls,
                    gSwappingBuffers,
                    pthread_self());
#endif 

        if(dfbNativeSurface)
        {
            // When data is provided copy into DFB surface, otherwise clear the surface.
            if ( data )
            {
                void *ret_ptr;
                int pitch;

                // Transfer the decoded image into Video memory. For ultimate performance requiring data to fit surface exactly
                DFBCHECK( (dfbNativeSurface)->Lock( dfbNativeSurface, DSLF_WRITE, &ret_ptr, &pitch ));
                ASSERT( pitch == (size.width() * sizeof(unsigned)) );
                memcpy( ret_ptr, data, size.width() * size.height() * sizeof(unsigned) );
                DFBCHECK( dfbNativeSurface->Unlock (dfbNativeSurface ) );
            }
            else
            {

            //    DFBCHECK( (dfbNativeSurface)->Clear( dfbNativeSurface, 0, 0, 0, 0 ) );

            }

        }

    }
    return dfbNativeSurface;
}
} // namespace WebCore

#endif // ENABLE(DIRECTFB)