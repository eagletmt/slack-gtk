project(slack-gtk CXX)
cmake_minimum_required(VERSION 3.4)

include(FindPkgConfig)
pkg_check_modules(libsoup REQUIRED libsoup-2.4)
include_directories(${libsoup_INCLUDE_DIRS})
link_directories(${libsoup_LIBRARY_DIRS})
link_libraries(${libsoup_LIBRARIES})

pkg_check_modules(jsoncpp REQUIRED jsoncpp>=1.0.0)
include_directories(${jsoncpp_INCLUDE_DIRS})
link_directories(${jsoncpp_LIBRARY_DIRS})
link_libraries(${jsoncpp_LIBRARIES})

pkg_check_modules(gtk REQUIRED gtkmm-3.0)
include_directories(${gtk_INCLUDE_DIRS})
link_directories(${gtk_LIBRARY_DIRS})
link_libraries(${gtk_LIBRARIES})

pkg_check_modules(libnotify REQUIRED libnotify)
include_directories(${libnotify_INCLUDE_DIRS})
link_directories(${libnotify_LIBRARY_DIRS})
link_libraries(${libnotify_LIBRARIES})

link_libraries(pthread)

add_definitions(-std=c++11 -Wall -W)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

include_directories("${CMAKE_SOURCE_DIR}/include")
set(SOURCES
  src/api_client.cc
  src/attachments_view.cc
  src/bottom_adjustment.cc
  src/channel_window.cc
  src/channels_store.cc
  src/emoji_loader.cc
  src/icon_loader.cc
  src/main.cc
  src/main_window.cc
  src/message_entry.cc
  src/message_row.cc
  src/message_text_view.cc
  src/rtm_client.cc
  src/users_store.cc
  )
add_executable(slack-gtk ${SOURCES})

install(PROGRAMS slack-gtk DESTINATION bin)

# gsettings
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  pkg_get_variable(glib_compile_schemas gio-2.0 glib_compile_schemas)
  set(gschema_source "${CMAKE_CURRENT_SOURCE_DIR}/data/cc.wanko.slack-gtk.gschema.xml")
  set(gschema_copy "schemas/cc.wanko.slack-gtk.gschema.xml")
  set(gschema_compiled "schemas/gschemas.compiled")
  add_custom_command(
    OUTPUT "${gschema_copy}"
    COMMAND mkdir -p schemas
    COMMAND cp "${gschema_source}" "${gschema_copy}"
    COMMENT "Copying gschema"
    DEPENDS "${gschema_source}"
  )
  add_custom_command(
    OUTPUT "${gschema_compiled}"
    COMMAND ${glib_compile_schemas} schemas
    COMMENT "Compiling gschema"
    DEPENDS "${gschema_copy}"
  )
  add_custom_target(compile-gschema ALL DEPENDS "${gschema_compiled}")
endif()
