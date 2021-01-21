add_library(libmdart OBJECT
    mdart.cc
    mdart.h
    mdart_adp.cc
    mdart_adp.h
    mdart_dht.cc
    mdart_dht.h
    mdart_function.h
    mdart_ndp.cc
    mdart_ndp.h
    mdart_neighbor.cc
    mdart_neighbor.h
    mdart_packet.h
    mdart_queue.cc
    mdart_queue.h
    mdart_table.cc
    mdart_table.h
        )
target_include_directories(libmdart PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )

