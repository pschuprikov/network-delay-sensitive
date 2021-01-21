add_library(libtora OBJECT
            tora.cc
            tora.h
            tora_api.cc
            tora_dest.cc
            tora_dest.h
            tora_io.cc
            tora_logs.cc
            tora_neighbor.cc
            tora_neighbor.h
            tora_packet.h
        )
target_include_directories(libtora PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(libtora PRIVATE libcommon)
target_compile_definitions(libtora INTERFACE WITH_TORA)
