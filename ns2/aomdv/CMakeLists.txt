add_library(libaomdv OBJECT
            aomdv.cc
            aomdv.h
            aomdv_logs.cc
            aomdv_packet.h
            aomdv_rqueue.cc
            aomdv_rqueue.h
            aomdv_rtable.cc
            aomdv_rtable.h
        )
target_include_directories(libaomdv PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:librouting,INTERFACE_INCLUDE_DIRECTORIES>
    )
target_link_libraries(libaomdv PUBLIC libmobile)
target_compile_definitions(libaomdv INTERFACE WITH_AOMDV)
