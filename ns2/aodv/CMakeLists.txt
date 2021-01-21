add_library(libaodv OBJECT
        aodv.h
        aodv.cc
        aodv_logs.cc
        aodv_packet.h
        aodv_rqueue.h
        aodv_rqueue.cc
        aodv_rtable.h
        aodv_rtable.cc
        )
target_include_directories(libaodv PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:librouting,INTERFACE_INCLUDE_DIRECTORIES>
    )
target_link_libraries(libaodv PUBLIC libmobile)
target_compile_definitions(libaodv INTERFACE WITH_AODV)
