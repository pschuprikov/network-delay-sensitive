add_library(librap OBJECT
           media-app.cc
           media-app.h
           rap.cc
           rap.h
           raplist.cc
           raplist.h
           utilities.cc
           utilities.h
        )
target_include_directories(librap PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
    )
target_compile_definitions(librap INTERFACE WITH_RAP)
