add_library(libasim OBJECT
        asim.h
        asim.cc
        )
target_include_directories(libasim PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
