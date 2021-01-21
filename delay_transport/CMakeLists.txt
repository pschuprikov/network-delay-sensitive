cmake_minimum_required(VERSION 3.13)
project(delay_transport)

option(WITH_ASAN "Enable address sanitizer")

set(CMAKE_CXX_STANDARD 17)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    add_compile_options(-ggdb -O0)
endif()

add_compile_options(-Wall -Wextra -Werror)

if(WITH_ASAN)
    add_compile_options(-fsanitize=address)
endif()

find_package(spdlog REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
add_subdirectory(src)

export(TARGETS delay_transport FILE delay-transport-exports.cmake)

install(TARGETS delay_transport 
        EXPORT DelayTransportTargets
        )

install(DIRECTORY include/delay_transport DESTINATION include)

install(EXPORT DelayTransportTargets
        FILE delay_transportConfig.cmake
        NAMESPACE delay_transport::
        DESTINATION lib/cmake/delay_transport/
        )
