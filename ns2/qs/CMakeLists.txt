add_library(libqs OBJECT
        hdr_qs.cc
        hdr_qs.h
        qsagent.cc
        qsagent.h
        )
target_include_directories(libqs PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )

target_compile_definitions(libqs INTERFACE WITH_QS)
