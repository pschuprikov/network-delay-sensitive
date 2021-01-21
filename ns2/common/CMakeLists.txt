add_library(libcommon OBJECT
        agent.cc
        agent.h
        bi-connector.cc
        bi-connector.h
        connector.cc
        connector.h
        Decapsulator.cc
        Decapsulator.h
        encap.cc
        encap.h
        Encapsulator.cc
        Encapsulator.h
        flags.h
        fsm.cc
        fsm.h
        heap.h
        ip.cc
        ip.h
        ivs.cc
        location.h
        message.cc
        message.h
        messpass.cc
        messpass.h
        misc.cc
        net-interface.cc
        net-interface.h
        node.cc
        node.h
        ns-process.cc
        ns-process.h
        object.cc
        object.h
        packet-stamp.h
        packet.cc
        packet.h
        parentnode.cc
        parentnode.h
        pkt-counter.cc
        scheduler-map.cc
        scheduler.cc
        scheduler.h
        session-rtp.cc
        sessionhelper.cc
        simulator.cc
        simulator.h
        splay-scheduler.cc
        timer-handler.cc
        timer-handler.h
        tp.cc
        tp.h
        tpm.cc
        tpm.h
        ttl.cc)

if(WITH_MOBILE)
    target_sources(libcommon PRIVATE
        mobilenode.cc
        mobilenode.h
    )
    target_link_libraries(libcommon PUBLIC libmobile)
endif()

target_compile_features(libcommon PUBLIC cxx_std_17)
target_include_directories(
        libcommon PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${TCLCL_INCLUDE_DIRS}
        ${TCL_INCLUDE_PATH}
        $<TARGET_PROPERTY:libtrace,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:libmac,INTERFACE_INCLUDE_DIRECTORIES>
)
