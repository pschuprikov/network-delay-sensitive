add_library(libwebcache OBJECT
            http.cc
            http.h
            http-aux.cc
            http-aux.h
            inval-agent.cc
            inval-agent.h
            logweb.cc
            logweb.h
            mcache.cc
            mcache.h
            pagepool.cc
            pagepool.h
            tcp-simple.cc
            tcp-simple.h
            tcpapp.cc
            tcpapp.h
            webserver.cc
            webserver.h
            webtraf.cc
            webtraf.h
        )
target_include_directories(libwebcache PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
