add_library(libwpan OBJECT
        p802_15_4const.h
        p802_15_4csmaca.cc
        p802_15_4csmaca.h
        p802_15_4def.h
        p802_15_4fail.cc
        p802_15_4fail.h
        p802_15_4field.h
        p802_15_4hlist.cc
        p802_15_4hlist.h
        p802_15_4mac.cc
        p802_15_4mac.h
        p802_15_4nam.cc
        p802_15_4nam.h
        p802_15_4phy.cc
        p802_15_4phy.h
        p802_15_4pkt.h
        p802_15_4sscs.cc
        p802_15_4sscs.h
        p802_15_4timer.cc
        p802_15_4timer.h
        p802_15_4trace.cc
        p802_15_4trace.h
        p802_15_4transac.cc
        p802_15_4transac.h
        )
target_include_directories(libwpan PUBLIC `${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(libwpan PRIVATE libcommon)
target_compile_definitions(libwpan INTERFACE WITH_WPAN)
