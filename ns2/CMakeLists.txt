cmake_minimum_required(VERSION 3.13)

project(ns2 LANGUAGES C CXX)

option(WITH_QS "enable qs")
option(WITH_NIX "enable nix")
option(WITH_MOBILE "enable mobile")
option(WITH_PGM "enable PGM")
option(WITH_RAP "enable RAP")
option(WITH_DSDV "enable DSDV")
option(WITH_ASIM "enable ASIM")
option(WITH_TMIX "enable TMIX")
option(WITH_DIFFUSION "enable DIFFUSION")
option(WITH_PACKMIME "enable PACKMIME")
option(WITH_TORA "enable TORA")
option(WITH_AODV "enable AODV")
option(WITH_AOMDV "enable AOMDV")
option(WITH_WPAN "enable WPAN")
option(WITH_DSR "enable DSR")
option(WITH_IMEP "enable IMEP")
option(WITH_DELAYBOX "enable DELAYBOX")
option(WITH_DELAY_CC "enable DELAY_CC" ON)
option(WITH_SCTP "enable SCTP")
option(LOCAL_DELAY_TRANSPORT "use delay transport from local repo")
option(WITH_ASAN "Enable address sanitizer")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CheckTypeSize)
include(CheckCXXSymbolExists)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

check_type_size(long SIZEOF_LONG)
check_cxx_symbol_exists(strtoll cstdlib HAVE_STRTOLL)
check_cxx_symbol_exists(snprintf cstdio HAVE_SNPRINTF)


add_compile_definitions(USE_INTERP_RESULT USE_INTERP_ERRORLINE)
add_compile_definitions(HAVE_ARPA_INET_H HAVE_STL)
add_compile_definitions("RANDOM_RETURN_TYPE=long int")
add_compile_definitions("SIZEOF_LONG=${SIZEOF_LONG}")
add_compile_definitions(rng_test)

if(DEFINED HAVE_STRTOLL)
    add_compile_definitions(HAVE_STRTOLL)
endif()
if(DEFINED HAVE_SNPRINTF)
    add_compile_definitions(HAVE_SNPRINTF)
endif()
add_compile_definitions(HAVE_INT64)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_options(-ggdb -O0)
endif()

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
add_compile_options(-Wall -Werror)
if(WITH_ASAN)
    add_compile_options(-fsanitize=address)
endif()

find_package(OTCL REQUIRED)
find_package(TCLCL REQUIRED)
find_package(TCL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(Threads REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR} 
    ${TCL_INCLUDE_PATH}
    ${OTCL_INCLUDE_DIRS})

add_subdirectory(adc)
if(WITH_AODV)
    add_subdirectory(aodv)
endif()
if(WITH_AOMDV)
    add_subdirectory(aomdv)
endif()
add_subdirectory(apps)
if(WITH_ASIM)
    add_subdirectory(asim)
endif()
add_subdirectory(classifier)
add_subdirectory(common)
#add_subdirectory(dccp)
if(WITH_DELAY_CC)
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_SUFFIX)
    if(LOCAL_DELAY_TRANSPORT)
        include("../../delay_transport/cmake-build-${BUILD_TYPE_SUFFIX}/delay-transport-exports.cmake")
    else()
        find_package(delay_transport REQUIRED)
    endif()
    add_subdirectory(delay_cc)
endif()
if(WITH_DELAYBOX)
    add_subdirectory(delaybox)
endif()
#add_subdirectory(diffserv)
if(WITH_DIFFUSION)
    add_subdirectory(diffusion)
endif()
if(WITH_DSDV)
    add_subdirectory(dsdv)
endif()
if(WITH_DSR)
    add_subdirectory(dsr)
endif()
#add_subdirectory(empweb)
#add_subdirectory(gaf)
if(WITH_IMEP)
    add_subdirectory(imep)
endif()
add_subdirectory(link)
add_subdirectory(linkstate)
add_subdirectory(mac)
add_subdirectory(mcast)
#add_subdirectory(mdart)
if(WITH_MOBILE)
    add_subdirectory(mobile)
endif()
#add_subdirectory(mpls)
if(WITH_NIX)
    add_subdirectory(nix)
    add_subdirectory(routealgo)
endif()
if(WITH_PACKMIME)
    add_subdirectory(packmime)
endif(WITH_PACKMIME)
if(WITH_PGM)
    add_subdirectory(pgm)
endif()
#add_subdirectory(plm)
#add_subdirectory(pushback)
if(WITH_QS)
    add_subdirectory(qs)
endif()
add_subdirectory(queue)
if(WITH_RAP)
    add_subdirectory(rap)
endif()
#add_subdirectory(realaudio)
add_subdirectory(routing)
#add_subdirectory(satellite)
if(WITH_SCTP)
    add_subdirectory(sctp)
endif()
add_subdirectory(tcp)
if(WITH_TMIX)
    add_subdirectory(tmix)
endif()
add_subdirectory(tools)
if(WITH_TORA)
    add_subdirectory(tora)
endif()
add_subdirectory(trace)
#add_subdirectory(webcache)
if(WITH_WPAN)
    add_subdirectory(wpan)
endif()
#add_subdirectory(xcp)

set(NS_TCL_LIB_STL
        ${CMAKE_CURRENT_SOURCE_DIR}/tcl/lib/ns-diffusion.tcl
        )
if(WITH_PACKMIME)
    list(APPEND NS_TCL_LIB_STL ${CMAKE_CURRENT_SOURCE_DIR}/tcl/packmime/packmime.tcl)
endif()
if(WITH_TMIX)
    list(APPEND NS_TCL_LIB_STL ${CMAKE_CURRENT_SOURCE_DIR}/tcl/tmix/tmix.tcl)
    if(WITH_DELAYBOX)
        list(APPEND NS_TCL_LIB_STL ${CMAKE_CURRENT_SOURCE_DIR}/tcl/tmix/tmix_delaybox.tcl)
    endif()
endif()
if(WITH_DELAYBOX)
    list(APPEND NS_TCL_LIB_STL ${CMAKE_CURRENT_SOURCE_DIR}/tcl/delaybox/delaybox.tcl)
endif()
if(WITH_DELAY_CC)
    list(APPEND NS_TCL_LIB_STL ${CMAKE_CURRENT_SOURCE_DIR}/tcl/delay_cc/delay_cc.tcl)
endif()

STRING(REPLACE ";" "	" NS_TCL_LIB_STL "${NS_TCL_LIB_STL}")

add_custom_command(
        OUTPUT version.c ns_tcl.cc ptypes.cc
        COMMAND ${TCL_TCLSH} ${CMAKE_CURRENT_SOURCE_DIR}/bin/string2c.tcl version_string < ${CMAKE_CURRENT_SOURCE_DIR}/VERSION > version.c
        COMMAND ./ptypes2tcl > ptypes.cc
        COMMAND ${TCL_TCLSH} ${CMAKE_CURRENT_SOURCE_DIR}/bin/tcl-expand.tcl ${CMAKE_CURRENT_SOURCE_DIR}/tcl/lib/ns-lib.tcl ${NS_TCL_LIB_STL} | ${TCLCL2CXX} et_ns_lib > ns_tcl.cc
        DEPENDS
            ptypes2tcl
            ${NS_TCL_LIB_STL}
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-default.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-lib.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-node.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-route.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-source.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-queue.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-agent.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-link.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lib/ns-packet.tcl
            ${CMAKE_SOURCE_DIR}/tcl/lan/ns-ll.tcl
        )

add_executable(ptypes2tcl common/ptypes2tcl.cc)
target_include_directories(ptypes2tcl PRIVATE
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
target_link_libraries(ptypes2tcl PRIVATE )

add_executable(ns
        common/main-monolithic.cc
        common/tclAppInit.cc
        version.c
        ns_tcl.cc
        ptypes.cc)
target_link_libraries(ns PRIVATE
        libadc
        libapps
        libclassifier
        libcommon
        #libdccp
        #libdiffserv
        #libempweb
        #libgaf
        liblinkstate
        libtools
        #libpushback
        #librealaudio
        librouting
        #libsatellite
        liblink
        libmac
        libmcast
        #libmdart
        #libmpls
        libqueue
        #libplm
        libtcp
        libtools
        libtrace
        #libwebcache
        #libxcp
        ${TCL_LIBRARY} ${TCLCL_LIBRARIES} ${OTCL_LIBRARIES}
    )

if(WITH_NIX)
    target_link_libraries(ns PRIVATE libnix libroutealgo)
endif()
if(WITH_QS)
    target_link_libraries(ns PRIVATE libqs)
endif()
if(WITH_MOBILE)
    target_link_libraries(ns PRIVATE libmobile)
endif()
if(WITH_PGM)
    target_link_libraries(ns PRIVATE libpgm)
endif()
if(WITH_RAP)
    target_link_libraries(ns PRIVATE librap)
endif()
if(WITH_DSDV)
    target_link_libraries(ns PRIVATE libdsdv)
endif()
if(WITH_ASIM)
    target_link_libraries(ns PRIVATE libasim)
endif()
if(WITH_PACKMIME)
    target_link_libraries(ns PRIVATE libpackmime)
endif()
if(WITH_TMIX)
    target_link_libraries(ns PRIVATE libtmix)
endif()
if(WITH_DIFFUSION)
    target_link_libraries(ns PRIVATE libdiffusion)
endif()
if(WITH_TORA)
    target_link_libraries(ns PRIVATE libtora)
endif()
if(WITH_AODV)
    target_link_libraries(ns PRIVATE libaodv)
endif()
if(WITH_AOMDV)
    target_link_libraries(ns PRIVATE libaomdv)
endif()
if(WITH_WPAN)
    target_link_libraries(ns PRIVATE libwpan)
endif()
if(WITH_DSR)
    target_link_libraries(ns PRIVATE libdsr)
endif()
if(WITH_IMEP)
    target_link_libraries(ns PRIVATE libimep)
endif()
if(WITH_DELAYBOX)
    target_link_libraries(ns PRIVATE libdelaybox)
endif()
if(WITH_DELAY_CC)
    target_link_libraries(ns PRIVATE libdelay_cc)
endif()
if(WITH_SCTP)
    target_link_libraries(ns PRIVATE libsctp)
endif()

if(WITH_ASAN)
    target_link_libraries(ptypes2tcl PRIVATE asan ${CMAKE_DL_LIBS} Threads::Threads rt)
    target_link_libraries(ns PRIVATE asan ${CMAKE_DL_LIBS} Threads::Threads rt)
endif()

install(TARGETS ns)
