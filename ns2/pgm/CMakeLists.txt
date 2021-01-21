add_library(libpgm OBJECT
        classifier-pgm.cc
        pgm.h
        pgm-agent.cc
        pgm-sender.cc
        pgm-receiver.cc
    )
target_include_directories(libpgm PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
    )
target_compile_definitions(libpgm INTERFACE WITH_PGM)
