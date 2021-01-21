add_library(libdsr OBJECT
            add_sr.cc
            constants.h
            dsr_proto.cc
            dsr_proto.h
            dsragent.cc
            dsragent.h
            flowstruct.cc
            flowstruct.h
            hdr_sr.cc
            hdr_sr.h
            linkcache.cc
            mobicache.cc
            path.cc
            path.h
            requesttable.cc
            requesttable.h
            routecache.cc
            routecache.h
            simplecache.cc
            sr_forwarder.cc
            sr_forwarder.h
            srpacket.h
            cache_stats.h
        )
target_include_directories(libdsr PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(libdsr PRIVATE libcommon)
target_compile_definitions(libdsr INTERFACE WITH_DSR)
