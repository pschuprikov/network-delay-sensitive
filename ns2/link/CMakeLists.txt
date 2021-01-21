add_library(liblink OBJECT
        delay.cc
        delay.h
        dynalink.cc
        hackloss.cc
        )
target_include_directories(liblink PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(liblink PUBLIC libcommon libmcast)
