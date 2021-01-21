add_library(libsctp OBJECT
        sctp-cmt.cc
        sctp-cmt.h
        sctp-hbAfterRto.cc
        sctp-hbAfterRto.h
        sctp-mfrHbAfterRto.cc
        sctp-mfrHbAfterRto.h
        sctp-mfrTimestamp.cc
        sctp-mfrTimestamp.h
        sctp-multipleFastRtx.cc
        sctp-multipleFastRtx.h
        #sctp-newreno.cc
        #sctp-newreno.h
        sctp-timestamp.cc
        sctp-timestamp.h
        sctp.cc
        sctp.h
        sctpDebug.cc
        sctpDebug.h
        )
target_include_directories(libsctp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
    )
target_compile_definitions(libsctp PUBLIC WITH_SCTP)
