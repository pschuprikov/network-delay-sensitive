add_library(libapps OBJECT
        app.cc
        app.h
        bitops.h
        pbc.cc
        pbc.h
        ping.cc
        ping.h
        rtp.cc
        rtp.h
        telnet.cc
        telnet.h
        udp.cc
        udp.h
        vat.h
        vatrcvr.cc
        worm.cc
        worm.h
        )
target_include_directories(libapps PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:librouting,INTERFACE_INCLUDE_DIRECTORIES>
    )
if(WITH_SCTP)
    target_sources(libapps PRIVATE
            sctp_app1.cc
            sctp_app1.h
        )
endif()
