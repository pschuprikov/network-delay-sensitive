add_library(libtrace OBJECT
        basetrace.cc
        basetrace.h
        trace-ip.cc
        trace.cc
        trace.h
        traffictrace.cc
        )
target_include_directories(libtrace PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:libapps,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:libtcp,INTERFACE_INCLUDE_DIRECTORIES>
    )
target_compile_features(libtrace PUBLIC 
    $<TARGET_PROPERTY:libcommon,INTERFACE_COMPLIE_FEATURES>)
if(WITH_SCTP)
    target_link_libraries(libtrace PRIVATE libsctp)
endif()
if(WITH_MOBILE)
    target_sources(libtrace PRIVATE
            cmu-trace.cc
            cmu-trace.h
        )
    target_link_libraries(libtrace PRIVATE libmobile)
endif()
if(WITH_RAP)
    target_link_libraries(libtrace PRIVATE librap)
endif()
if(WITH_DSR)
    target_link_libraries(libtrace PRIVATE libdsr)
endif()
if(WITH_IMEP)
    target_link_libraries(libtrace PRIVATE libimep)
endif()
if(WITH_TORA)
    target_link_libraries(libtrace PRIVATE libtora)
endif()
if(WITH_AODV)
    target_link_libraries(libtrace PRIVATE libaodv)
endif()
if(WITH_AOMDV)
    target_link_libraries(libtrace PRIVATE libaomdv)
endif()
if(WITH_DIFFUSION)
    target_link_libraries(libtrace PRIVATE libdiffusion)
endif()
if(WITH_WPAN)
    target_link_libraries(libtrace PRIVATE libwpan)
endif()
