add_library(libclassifier OBJECT
        classifier-addr.cc
        classifier-addr.h
        classifier-bst.cc
        classifier-hash.cc
        classifier-hash.h
        classifier-hier.cc
        classifier-hier.h
        classifier-mac.cc
        classifier-mcast.cc
        classifier-mcast.h
        classifier-mpath.cc
        classifier-port.cc
        classifier-port.h
        classifier-virtual.cc
        classifier.cc
        classifier.h
        filter.cc
        filter.h
        )

if(WITH_QS)
    target_sources(libclassifier PRIVATE
        classifier-qs.cc
        classifier-qs.h
    )
endif()
target_include_directories(libclassifier PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
