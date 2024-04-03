cmake_minimum_required(VERSION 3.9)

project(hadev 
  VERSION "1.0.0" 
  DESCRIPTION "Home Assistant Dvice"
  HOMEPAGE_URL "innovasoft.org"
)

# Collect the sources
add_executable(hadev
  main.c
  utils.c
)

target_link_libraries(hadev
  ${CMAKE_SOURCE_DIR}/../../lib/libmqttcli.a
)
