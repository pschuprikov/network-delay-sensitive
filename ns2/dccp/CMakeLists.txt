add_library(
    libdccp OBJECT
    bsd_queue.h
    dccp.cc
    dccp.h
    dccp_ackv.cc
    dccp_ackv.h
    dccp_opt.cc
    dccp_opt.h
    dccp_packets.cc
    dccp_packets.h
    dccp_sb.cc
    dccp_sb.h
    dccp_tcplike.cc
    dccp_tcplike.h
    dccp_tfrc.cc
    dccp_tfrc.h
    dccp_types.h
    )
target_include_directories(libdccp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
