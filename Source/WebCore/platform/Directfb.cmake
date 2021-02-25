if (USE_DIRECTFB)
    list(APPEND WebCore_INCLUDE_DIRECTORIES
        "${WEBCORE_DIR}/platform/graphics/directfb"
    )

    list(APPEND WebCore_SOURCES
        platform/graphics/directfb/DirectfbUtilities.cpp
        platform/graphics/directfb/DataCache.cpp
    )
endif ()

list(APPEND WebCore_SYSTEM_INCLUDE_DIRECTORIES
    ${DIRECTFB_INCLUDE_DIRS}
)

list(APPEND WebCore_LIBRARIES
    ${DIRECTFB_LIBRARIES}
)