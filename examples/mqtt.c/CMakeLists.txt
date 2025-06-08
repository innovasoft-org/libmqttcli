cmake_minimum_required(VERSION 3.9)

project(mqtt 
  VERSION "1.0.1" 
  DESCRIPTION "MQTT Client Program"
  HOMEPAGE_URL "innovasoft.org"
)

# Enable Threads
find_package (Threads)

# Enable OpenSSL
find_package(OpenSSL REQUIRED)

# Collect the sources
add_executable(mqtt
  main.c
  utils.c
)

target_link_libraries(mqtt 
  OpenSSL::SSL
)

target_link_libraries(mqtt
  ${CMAKE_THREAD_LIBS_INIT}
  ${CMAKE_SOURCE_DIR}/../../lib/libmqttcli.a
)
