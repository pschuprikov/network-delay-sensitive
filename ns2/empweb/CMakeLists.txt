add_library(libempweb OBJECT
            empftp.cc
            empftp.h
            empweb.cc
            empweb.h
            persconn.h
        )
target_include_directories(libempweb PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
