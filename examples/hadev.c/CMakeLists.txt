cmake_minimum_required(VERSION 3.9)

project(hadev 
  VERSION "1.0.1" 
  DESCRIPTION "Home Assistant Dvice Simulator"
  HOMEPAGE_URL "innovasoft.org"
)

# Enable Threads
find_package (Threads)

# Collect the sources
add_executable(hadev
  main.c
  utils.c
)

target_link_libraries(hadev
  ${CMAKE_THREAD_LIBS_INIT}
  ${CMAKE_SOURCE_DIR}/../../lib/libmqttcli.a
)
