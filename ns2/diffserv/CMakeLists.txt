add_library(libdiffserv OBJECT
            dewp.cc
            dewp.h
            dsCore.cc
            dsCore.h
            dsEdge.cc
            dsEdge.h
            dsPolicy.cc
            dsPolicy.h
            dsred.cc
            dsred.h
            dsredq.cc
            dsredq.h
            ew.cc
            ew.h
        )
target_include_directories(libdiffserv PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
