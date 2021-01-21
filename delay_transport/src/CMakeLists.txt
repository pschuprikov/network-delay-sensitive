add_library(delay_transport SHARED
        ../include/delay_transport/AnalysisMetadata.h
        ../include/delay_transport/DropInfo.h
        ../include/delay_transport/Environment.h
        ../include/delay_transport/Error.h
        ../include/delay_transport/PacketPropertiesAssigner.h
        ../include/delay_transport/PacketPropertiesAssignerFactory.h
        ../include/delay_transport/RawLogger.h
        ../include/delay_transport/ReceptionInfo.h
        AnalysisMetadataImpl.h
        Analyzer.cpp
        Analyzer.h
        DelayTransport.cpp
        DelayTransportImpl.cpp
        DelayTransportImpl.h
        Error.cpp
        ExpirationListener.h
        InboundMessage.cpp
        InboundMessage.h
        OutboundMessage.cpp
        OutboundMessage.h
        PacketPropertiesAssignerFactory.cpp
        )
target_include_directories(delay_transport PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(delay_transport PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/delay_transport>)
target_link_libraries(delay_transport PUBLIC spdlog::spdlog)

if(WITH_ASAN)
    target_link_libraries(delay_transport INTERFACE 
        rt 
        Threads::Threads 
        asan 
        ${CMAKE_DL_LIBS})
endif()
