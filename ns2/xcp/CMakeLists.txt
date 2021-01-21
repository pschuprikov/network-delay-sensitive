add_library(libxcp OBJECT
            xcp.cc
            xcp.h
            xcp-end-sys.cc
            xcp-end-sys.h
            xcpq.cc
            xcpq.h
        )
target_include_directories(libxcp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
