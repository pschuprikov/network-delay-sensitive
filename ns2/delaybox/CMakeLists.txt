add_library(libdelaybox OBJECT
        delaybox.cc
        delaybox.h
    )
target_include_directories(libdelaybox PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(libdelaybox PUBLIC libcommon)
