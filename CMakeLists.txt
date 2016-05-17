project(slack-gtk CXX)
cmake_minimum_required(VERSION 2.8)

include(FindPkgConfig)
pkg_check_modules(libsoup REQUIRED libsoup-2.4 libssl libcrypto)
include_directories(${libsoup_INCLUDE_DIRS})
link_directories(${libsoup_LIBRARY_DIRS})
link_libraries(${libsoup_LIBRARIES})

pkg_check_modules(jsoncpp REQUIRED jsoncpp)
include_directories(${jsoncpp_INCLUDE_DIRS})
link_directories(${jsoncpp_LIBRARY_DIRS})
link_libraries(${jsoncpp_LIBRARIES})

find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
link_libraries(${Boost_LIBRARIES})
link_libraries(pthread)

find_package(websocketpp REQUIRED)
include_directories(${WEBSOCKETPP_INCLUDE_DIR})

pkg_check_modules(gtk REQUIRED gtkmm-3.0)
include_directories(${gtk_INCLUDE_DIRS})
link_directories(${gtk_LIBRARY_DIRS})
link_libraries(${gtk_LIBRARIES})

add_definitions(-std=c++11 -Wall -W)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")

include_directories("${CMAKE_SOURCE_DIR}/include")
set(SOURCES
  src/main.cc
  src/api_client.cc
  src/rtm_client.cc
  src/rtm_client_impl.cc
  src/rtm_message_handler.cc
  src/main_window.cc
  src/message_entry.cc
  )
add_executable(slack-gtk ${SOURCES})

install(PROGRAMS slack-gtk DESTINATION bin)
