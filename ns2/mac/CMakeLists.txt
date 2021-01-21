add_library(libmac OBJECT
        arp.cc
        arp.h
        channel.cc
        channel.h
        lanRouter.cc
        lanRouter.h
        ll.cc
        ll.h
        mac-802_3.cc
        mac-802_3.h
        mac-simple.cc
        mac-simple.h
        mac.cc
        mac.h
        marshall.h
        phy.cc
        phy.h
        varp.cc
        varp.h
        wired-phy.cc
        wired-phy.h
        )

if(WITH_MOBILE)
    target_sources(libmac PRIVATE
        mac-timers.cc
        mac-timers.h
        wireless-phy.cc
        wireless-phy.h
        wireless-phyExt.cc
        wireless-phyExt.h
        mac-802_11.cc
        mac-802_11.h
        mac-802_11Ext.cc
        mac-802_11Ext.h
        mac-tdma.cc
        mac-tdma.h
        smac.cc
        smac.h
    )
    target_link_libraries(libmac PRIVATE libmobile)
endif()

target_include_directories(libmac PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:libtools,INTERFACE_INCLUDE_DIRECTORIES>
        )
target_link_libraries(libmac PRIVATE libqueue)
