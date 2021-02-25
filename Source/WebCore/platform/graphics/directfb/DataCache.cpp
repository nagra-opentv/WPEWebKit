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

#include "DataCache.h"
#include <glib-object.h>
#include <wtf/Assertions.h>

#define CACHE_DEBUG 0   /* set to 1 to turn on debug print */
#define INSERT_SORTED 0 /* if set to 0, the items will be prepended to the cache, */
                        /* if set to 1, the items will be sorted using the m_keyCompareFunc */

namespace WebCore
{
    
#if CACHE_DEBUG    
#define CACHE_DATA_PRINT_DBG(msg,args...) printf(msg, ## args)
#else
#define CACHE_DATA_PRINT_DBG(msg,args...)
#endif


#define INIT_GTYPE()       g_type_init()

/** Cache data struct.
  */
typedef struct
{
    gpointer key;  /* key used by the DataCompareFunc to insert the data sorted into cache.*/
    gpointer data; /* data to be inserted into cache. */
} CacheData_t;


/** Constructor
  * @param[in] max_cache_items  max number of items in cache. 
  * @param[in] comp_func  Function to be used for sort. 
  * @param[in] del_func  Function to be used for deletion of data. 
  *  Initialize the cache.
  */
DataCache::DataCache(guint max_cache_items, KeyCompareFunc key_comp_func, DeleteFunc key_del_func, DeleteFunc data_del_func)
    : m_keyCompareFunc(key_comp_func)
    , m_keyDeleteFunc(key_del_func)
    , m_dataDeleteFunc(data_del_func)
    , m_dumpCacheDataFunc(NULL)
	, m_dataList(NULL)
	, m_maxCacheItems(max_cache_items)
	, m_numCacheItems(0)
{
    INIT_GTYPE();
}

/** Destructor
  *  Delete all the items in the cache.
  */
DataCache::~DataCache()
{
    CACHE_DATA_PRINT_DBG("CometDataCache dtor\n");
    if(m_dataList)
    {
        CacheData_t *cache_data_p;
        for(guint i = 0; i < m_numCacheItems; i++)
        {
            cache_data_p = (CacheData_t*)g_list_nth_data(m_dataList, i);
            /* delete the key and data*/
            if(m_keyDeleteFunc)
                m_keyDeleteFunc(cache_data_p->key);
            if(m_dataDeleteFunc)
                m_dataDeleteFunc(cache_data_p->data);
            g_free(cache_data_p);
        }
        g_list_free(m_dataList);
    }
}

/** Function to flush the cache.
  * The DeleteFunc is called for every key and data element in the cache
  * @return None. 
  */
void DataCache::flushCache()
{
    CACHE_DATA_PRINT_DBG("DataCache Flush\n");
    if(m_dataList)
    {
        CacheData_t *cache_data_p;
        for(guint i = 0; i < m_numCacheItems; i++)
        {
            cache_data_p = (CacheData_t*)g_list_nth_data(m_dataList, i);
            /* delete the key and data*/
            if(m_keyDeleteFunc)
                m_keyDeleteFunc(cache_data_p->key);
            if(m_dataDeleteFunc)
                m_dataDeleteFunc(cache_data_p->data);
            g_free(cache_data_p);
        }
        g_list_free(m_dataList);
        m_numCacheItems = 0;
        m_dataList = NULL;
    }
}

/** Set the dump function implemented by caller to dump the cache data -to be used for debugging.
  * @param[in] dump_func the dump function implemented by caller to dump the cache data.
  *
  * @return None. 
  */
void DataCache::setDumpCacheDataFunc(DumpCacheDataFunc dump_cache_data_func)
{
    m_dumpCacheDataFunc = dump_cache_data_func;
}

/** Function to dump the cache -to be used for debugging.
  *    The DumpCacheDataFunc function is called for every element in the cache.
  * @return None. 
  */
void DataCache::dumpCacheData(void)
{
    if(m_dumpCacheDataFunc)
    {
        if(m_dataList)
        {
            CacheData_t *cache_data_p;
            for(guint i = 0; i < m_numCacheItems; i++)
            {
                cache_data_p = (CacheData_t*)g_list_nth_data(m_dataList, i);
                m_dumpCacheDataFunc(cache_data_p->key, cache_data_p->data);
            }
        }
    }
}

/** Insert data into cache.
  * @param[in] key If a DataCompareFunc is set, this key will be used to insert the data sorted into cache.
  * @param[in] data  Data to be inserted into cache.
  * If the cache is full it will delete the last item and insert the new item.
  * @return 0 on success, -1 on failure, 1 if the cache is full. 
  */
gint DataCache::insertDataIntoCache(gpointer key, gpointer data)
{
    if(key && m_keyCompareFunc)
    {
        CacheData_t *cache_data_p, *new_cache_data_p;

        if(m_numCacheItems == m_maxCacheItems)
        {
            /* the cache is full, delete the last item and then insert the new item sorted. */
            CACHE_DATA_PRINT_DBG("DataCache::insertDataIntoCache Cache is FULL, replace last\n");
            cache_data_p = (CacheData_t*)g_list_nth_data(m_dataList, m_numCacheItems-1);
            m_dataList = g_list_remove(m_dataList, cache_data_p);
            m_numCacheItems--;
            if(m_keyDeleteFunc)
                m_keyDeleteFunc(cache_data_p->key);
            if(m_dataDeleteFunc)
                m_dataDeleteFunc(cache_data_p->data);
            new_cache_data_p = cache_data_p;
        }
        else
        {
            new_cache_data_p = g_new0(CacheData_t, 1);
        }
        
        new_cache_data_p->key = key;
        new_cache_data_p->data = data;

#if INSERT_SORTED        
        if(m_dataList == NULL)
        {
            /* First item: prepend it to the list */
            m_dataList = g_list_prepend(m_dataList, (gpointer)new_cache_data_p);
            m_numCacheItems++;
            CACHE_DATA_PRINT_DBG("DataCache::insertDataIntoCache first key=%p, data=%p num_items=%d\n",key, data, m_numCacheItems);
            return 0;
        }
        else
        {
            gint cmp_result;
            GList *list_elem;

            for(guint i = 0; i < m_numCacheItems; i++)
            {
                list_elem = g_list_nth(m_dataList, i);
                cache_data_p = (CacheData_t*)list_elem->data;
                cmp_result =  m_keyCompareFunc(key, cache_data_p->key);
                if(cmp_result <= 0) 
                {
                    /* the new item is to be inserted before the current item */
                    m_dataList = g_list_insert_before(m_dataList, list_elem, new_cache_data_p);
                    m_numCacheItems++;
                    CACHE_DATA_PRINT_DBG("DataCache::insertDataIntoCache key=%p, data=%p num_items=%d\n",key, data, m_numCacheItems);
                    return 0;
                }
            }
            /* the new item is to be inserted at the end */
            m_dataList = g_list_append(m_dataList, new_cache_data_p);
            m_numCacheItems++;
            CACHE_DATA_PRINT_DBG("DataCache::insertDataIntoCache key=%p, data=%p num_items=%d\n",key, data, m_numCacheItems);
            return 0;
        }
#else /* INSERT_SORTED */
        m_dataList = g_list_prepend(m_dataList, (gpointer)new_cache_data_p);
        m_numCacheItems++;
        CACHE_DATA_PRINT_DBG("DataCache::insertDataIntoCache key=%p, data=%p num_items=%d\n",key, data, m_numCacheItems);
        return 0;
#endif /* INSERT_SORTED */
    }
    
    CACHE_DATA_PRINT_DBG("DataCache::insertDataIntoCache returns failure\n");
    return -1;
}

/** Find cached data using key.
  * @param[in] key If a DataCompareFunc is set, this key will be used to find the data stored in cache.
  * If the cached item is found, its key is deleted and data returned.
  * @return cached data if found else NULL. 
  */
gpointer DataCache::findCachedData(gpointer key)
{  
    if(key && m_keyCompareFunc)
    {
        CacheData_t *cache_data_p;
        gpointer data;
        gint cmp_result;
        for(guint i = 0; i < m_numCacheItems; i++)
        {
            cache_data_p = (CacheData_t*)g_list_nth_data(m_dataList, i);
            cmp_result =  m_keyCompareFunc(key, cache_data_p->key);
            if(cmp_result == 0) 
            {
                m_dataList = g_list_remove(m_dataList, cache_data_p);
                m_numCacheItems--;
                data = cache_data_p->data;
                if(m_keyDeleteFunc)
                    m_keyDeleteFunc(cache_data_p->key);
                g_free(cache_data_p);
                return data;
            }
        }
    }

    CACHE_DATA_PRINT_DBG("DataCache::findCachedData returns NULL\n");
    return NULL;   
}

guint DataCache::size(void)
{
    return m_numCacheItems;
}


} // namespace WebCore

