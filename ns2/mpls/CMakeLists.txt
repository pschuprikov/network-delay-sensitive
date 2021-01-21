add_library(libmpls OBJECT
            classifier-addr-mpls.cc
            classifier-addr-mpls.h
            ldp.cc
            ldp.h
            mpls-module.cc
            mpls-module.h
        )
target_include_directories(libmpls PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
