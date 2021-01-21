add_library(libimep OBJECT
            dest_queue.cc
            dest_queue.h
            imep.cc
            imep.h
            imep_api.cc
            imep_io.cc
            imep_rt.cc
            imep_spec.h
            imep_stats.h
            imep_timers.cc
            imep_util.cc
            rxmit_queue.cc
            rxmit_queue.h
        )
target_include_directories(libimep PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
    )
target_compile_definitions(libimep INTERFACE WITH_IMEP)
