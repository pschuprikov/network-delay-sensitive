add_library(librealaudio OBJECT
        realaudio.cc
        )
target_include_directories(librealaudio PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
