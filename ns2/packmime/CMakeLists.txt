add_library(libpackmime OBJECT
            packmime_HTTP.cc
            packmime_HTTP.h
            packmime_HTTP_rng.cc
            packmime_OL.cc
            packmime_OL.h
            packmime_OL_ranvar.cc
            packmime_OL_ranvar.h
            packmime_ranvar.cc
            packmime_ranvar.h
        )
target_include_directories(libpackmime PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
