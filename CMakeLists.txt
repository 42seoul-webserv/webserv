cmake_minimum_required(VERSION 3.13)

project("webserv")

set(CMAKE_CXX_STANDARD 98)

add_compile_options(-g3 -fsanitize=address)
add_link_options(-g3 -fsanitize=address)

set(SOURCE_FILES
        src/main.cpp
        src/RequestParser.cpp
        src/ServerUtil.cpp
        src/ServerManager.cpp
        src/Server.cpp
        src/Parser.cpp
        src/HTTPResponse.cpp
        src/RequestProcessor.cpp
        src/Location.cpp
        src/ThreadPool.cpp
        )

add_executable(webserv
        ${SOURCE_FILES}
        )

target_include_directories(webserv
        PUBLIC
        include
        )
