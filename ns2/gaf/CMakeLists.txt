add_library(libgaf OBJECT
        gaf.cc
        gaf.h
        )
target_include_directories(libgaf PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
    )
target_link_libraries(libgaf PUBLIC libcommon)
