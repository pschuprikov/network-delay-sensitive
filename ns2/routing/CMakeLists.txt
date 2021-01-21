add_library(librouting OBJECT
        addr-params.cc
        addr-params.h
        address.cc
        address.h
        alloc-address.cc
        route.cc
        route.h
        rtmodule.cc
        rtmodule.h
        rtProtoDV.cc
        rtProtoDV.h
        rttable.cc
        rttable.h
        )
target_include_directories(librouting PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:libclassifier,INTERFACE_INCLUDE_DIRECTORIES>
        )
