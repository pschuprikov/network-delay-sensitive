add_library(libdsdv OBJECT
            dsdv.cc
            dsdv.h
            rtable.cc
            rtable.h
        )
target_include_directories(libdsdv PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
target_link_libraries(libdsdv PRIVATE libcommon)
