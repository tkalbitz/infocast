cmake_minimum_required(VERSION 2.8)
project(infocast)

FIND_PACKAGE( Boost 1.45 COMPONENTS program_options serialization-mt REQUIRED )

include_directories(
	../include/
	../lib/
	${INCLUDE_PREFIX}
        ${Boost_INCLUDE_DIRS}
)

set(_srcs
    say_thread.cpp
    infocast_sensors.cpp
    infocast_network.cpp
    infocast.cpp
)

# Create a plugin, that is a shared library, and make
# sure it is built in lib/naoqi
add_executable(infocast ${_srcs})
target_link_libraries(infocast alcommon alvalue alerror alproxies pthread ${Boost_LIBRARIES})
