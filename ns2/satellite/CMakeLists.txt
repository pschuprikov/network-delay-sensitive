add_library(libsatellite OBJECT
            sat-hdlc.cc
            sat-hdlc.h
            satgeometry.cc
            satgeometry.h
            sathandoff.cc
            sathandoff.h
            satlink.cc
            satlink.h
            satnode.cc
            satnode.h
            satposition.cc
            satposition.h
            satroute.cc
            satroute.h
            sattrace.cc
            sattrace.h
        )
target_include_directories(libsatellite PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
