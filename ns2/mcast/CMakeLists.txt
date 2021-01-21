add_library(libmcast OBJECT
        classifier-lms.cc
        ctrMcast.cc
        ctrMcast.h
        lms-agent.cc
        lms-receiver.cc
        lms-sender.cc
        lms.h
        mcast_ctrl.cc
        mcast_ctrl.h
        rcvbuf.cc
        rcvbuf.h
        replicator.cc
        #rlm.cc
        srm-headers.h
        srm-ssm.cc
        srm-ssm.h
        srm-state.h
        srm-topo.cc
        srm-topo.h
        srm.cc
        srm.h
        )
target_include_directories(libmcast PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
