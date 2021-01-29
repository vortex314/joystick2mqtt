cmake_minimum_required(VERSION 3.0.0)
project(joystick2mqtt VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(joystick2mqtt src/Main.cpp src/Joystick.cpp) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(GCC_FLAGS "-DLINUX")
include(CPack)

add_definitions(${GCC_FLAGS})
include_directories(src  ../limero/inc ../limero/linux ../ArduinoJson/src ../paho.mqtt.c/src  )
target_link_libraries(joystick2mqtt -lpthread -L../../limero/linux/build/ -llimero -L../../paho.mqtt.c/build/output/ -l:libpaho-mqtt3c.a  -lrt -pthread -latomic )
EXECUTE_PROCESS( COMMAND arch OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE )

if ( ${ARCH} STREQUAL "armv6l" OR ${ARCH} STREQUAL "armv7l")
        target_link_libraries(joystick2mqtt -lwiringPi)
	set(GCC_FLAGS "-DHAS_GPIO")
endif()

