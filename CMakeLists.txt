cmake_minimum_required(VERSION 3.13)

project("webserv")

set(CMAKE_CXX_STANDARD 98)

set(SOURCE_FILES
        src/Server.cpp
        src/main.cpp
        src/ServerManager.cpp
        src/ServerUtil.cpp)

add_executable(webserv
        ${SOURCE_FILES}
        )

target_include_directories(webserv
        PUBLIC
        include
        )