cmake_minimum_required(VERSION 3.0.0)
project(joystick2mqtt VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g ")
add_definitions(-DLINUX -std=c++11)
# set(CMAKE_BUILD_TYPE Debug)

set(WS /home/lieven/workspace )
set(COMPONENTS ${WS}/components)
set(HIREDIS ${WS}/hiredis )
set(JSON ${WS}/json)



add_executable(joystick2mqtt) 
add_executable(joystick2redis)

target_sources(joystick2redis PRIVATE 
    src/joystickRedis.cpp 
    src/Joystick.cpp 
    src/LogFile.cpp
    ${COMPONENTS}/src/StringUtility.cpp
    ${COMPONENTS}/linux/BrokerRedisJson.cpp
    ${COMPONENTS}/linux/Log.cpp
    ${COMPONENTS}/linux/Sys.cpp
    ${COMPONENTS}/linux/limero.cpp
    ) 


target_sources(joystick2mqtt PRIVATE 
    src/Main.cpp 
    src/Joystick.cpp 
    src/LogFile.cpp
    ${COMPONENTS}/src/StringUtility.cpp
    ${COMPONENTS}/linux/Log.cpp
    ${COMPONENTS}/linux/Sys.cpp
    ${COMPONENTS}/linux/limero.cpp
    ${COMPONENTS}/linux/MqttPaho.cpp
    ) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(GCC_FLAGS "-DLINUX")
include(CPack)

add_definitions(${GCC_FLAGS})

include_directories(src  
        ../components/inc 
        ../components/linux 
        ../ArduinoJson/src 
        ../paho.mqtt.c/src  
        ${HIREDIS}
        ${JSON}/single_include
        )

target_link_libraries(joystick2mqtt 
        -lpthread 
        -L../../paho.mqtt.c/build/output/ -l:libpaho-mqtt3c.a  
        -lrt 
        -pthread 
        -latomic )

target_link_libraries(joystick2redis 
        -lpthread 
        -L${HIREDIS} -l:libhiredis.a
        -lrt 
        -pthread 
        -latomic )

EXECUTE_PROCESS( COMMAND arch OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE )

if ( ${ARCH} STREQUAL "armv6l" OR ${ARCH} STREQUAL "armv7l")
        target_link_libraries(joystick2mqtt -lwiringPi)
	set(GCC_FLAGS "-DHAS_GPIO")
endif()

