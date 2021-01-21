add_library(libtmix OBJECT
            tmix.cc
            tmix.h
            tmixAgent.h
            tmixAgent.cc
        )
target_include_directories(libtmix PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
if(WITH_DELAYBOX)
    target_sources(libtmix PRIVATE
            tmix_delaybox.cc
            tmix_delaybox.h
        )
endif()
