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

#ifndef OPENTV_DATACACHE_H
#define OPENTV_DATACACHE_H

#include <glib.h>

namespace WebCore {

/** Comparison function implemented by caller to compare two key values.
  * @param[in] keyA First key value to compare
  * @param[in] keyB Second key value to compare
  *
  * @return  a negative integer if the first key value comes before the second, 
  *    0 if they are equal, or a positive integer if the first value comes after the second.
  */
typedef gint (*KeyCompareFunc) (gconstpointer keyA, gconstpointer keyB);

/** Delete function implemented by caller to delete data previously inserted into cache.
  * @param[in] data  Data to be deleted from cache.
  *
  * @return None. 
  */
typedef void (*DeleteFunc) (gpointer data);

/** Function implemented by caller to dump the cache data -to be used for debugging.
  * @param[in] key the key associated with the data.
  * @param[in] data  Data to be dumped.
  *
  * @return None. 
  */
typedef void (*DumpCacheDataFunc) (gpointer key, gpointer data);

class DataCache
{
public:
    DataCache(guint max_cache_items, KeyCompareFunc comp_func, DeleteFunc key_del_func, DeleteFunc data_del_func);
    ~DataCache();

    /** Insert data into cache.
         * @param[in] key If a DataCompareFunc is set, this key will be used to insert the data sorted into cache.
         * @param[in] data  Data to be inserted into cache.
         *
         * @return 0 on success, -1 on failure, 1 if the cache is full. 
         */
    gint insertDataIntoCache(gpointer key, gpointer data);

    /** Find cached data using key.
         * @param[in] key If a DataCompareFunc is set, this key will be used to find the data stored in cache.
         *
         * @return cached data if found else NULL. 
         */
    gpointer findCachedData(gpointer key);

    /** Set the dump function implemented by caller to dump the cache data -to be used for debugging.
        * @param[in] dump_func the dump function implemented by caller to dump the cache data.
        *
        * @return None. 
        */
    void setDumpCacheDataFunc(DumpCacheDataFunc dump_func);

    /** Function to dump the cache -to be used for debugging.
        *    The DumpCacheDataFunc function is called for every element in the cache.
        * @return None. 
        */
    void dumpCacheData(void);

    /** Function to flush the cache.
        * The DeleteFunc is called for every key and data element in the cache
        * @return None. 
        */
    void flushCache(void);

    guint size(void);

private:
    KeyCompareFunc m_keyCompareFunc;
    DeleteFunc m_keyDeleteFunc;
    DeleteFunc m_dataDeleteFunc;
    DumpCacheDataFunc m_dumpCacheDataFunc;
    GList *m_dataList;
    guint m_maxCacheItems;
    guint m_numCacheItems;
        
};

} // WebCore

#endif


