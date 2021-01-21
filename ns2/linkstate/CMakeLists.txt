add_library(liblinkstate OBJECT
            hdr-ls.cc
            hdr-ls.h
            ls.cc
            ls.h
            rtProtoLS.cc
            rtProtoLS.h
        )
target_include_directories(liblinkstate PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
