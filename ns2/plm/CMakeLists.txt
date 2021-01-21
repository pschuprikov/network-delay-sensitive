add_library(libplm OBJECT
        cbr-traffic-PP.cc
        loss-monitor-plm.cc
        )
target_include_directories(libplm PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
