add_library(libdelay_cc OBJECT
        DelayTransport.h
        DelayTransport.cpp
        hdr_delay.h
        hdr_delay.cpp
        PointerPeer.cpp
        PointerPeer.h
        TimerAdapter.cpp
        TimerAdapter.h
        queue/PriorityEDFQueue.cpp
        queue/PriorityEDFQueue.h
        DelayAssignment/DelayAssigner.h
        DelayAssignment/FileDelayAssigner.cpp
        DelayAssignment/FileDelayAssigner.h
        DelayAssignment/PIASDelayAssigner.cpp
        DelayAssignment/PIASDelayAssigner.h
        DelayAssignment/FixedDelayAssigner.h
        DelayAssignment/FixedDelayAssigner.cpp
        DelayAssignment/DelayAssigner.cpp
        DelayAssignment/ExponentialDelayAssigner.cpp
        DelayAssignment/ExponentialDelayAssigner.h
        DelayAssignment/TopologyOracle.h
        DelayAssignment/MinDelayAssigner.cpp
        DelayAssignment/MinDelayAssigner.h
        DelayAssignment/GoodputDelayAssigner.cpp
        DelayAssignment/GoodputDelayAssigner.h
        queue/priority-d.cpp
        queue/priority-d.h
        queue/utils/ExpirationTimeControllerImpl.cpp
        queue/utils/ExpirationTimeControllerImpl.h
        queue/utils/ExpirationTimeController.h
        queue/utils/ExpirationTracker.h
        queue/utils/ExpirationTracker.cpp
        queue/utils/LastDelayController.cpp
        queue/utils/LastDelayController.h
        queue/FIFOEDFQueue.cpp
        queue/FIFOEDFQueue.h
        Logger.h
        DelayTransport_fwd.h
        AgentInfoProvider.h
        queue/LAQueue.h
        queue/LAQueue.cpp
        queue/mixins/DropInfoMixin.hpp
        queue/mixins/DropInfoMixin.h
        queue/mixins/ExpirationTrackingMixin.hpp
        queue/mixins/ExpirationTrackingMixin.h
        queue/drop-tail-d.cpp
        queue/drop-tail-d.h
        tcp/mixins/BindCachingMixin.h
        tcp/mixins/BindCachingMixin.hpp
        tcp/tcp-full-delay.cc
        tcp/DFullTcpAgent.h
        tcp/DFullTcpAgent.hpp
        DelayAssignment/UniformDelayAssigner.cpp 
        DelayAssignment/UniformDelayAssigner.h
        )
target_compile_features(libdelay_cc PRIVATE cxx_std_17)
target_link_libraries(libdelay_cc PUBLIC libcommon delay_transport)

target_link_libraries(libdelay_cc PUBLIC delay_transport)
