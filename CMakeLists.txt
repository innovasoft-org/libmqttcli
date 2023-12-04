cmake_minimum_required(VERSION 3.9)

project(mqtt 
  VERSION "1.0.0" 
  DESCRIPTION "MQTT Client Program"
  HOMEPAGE_URL "innovasoft.org"
)

# Collect the sources
add_executable(mqtt
  src/main.c
  src/utils.c
)

target_link_libraries(mqtt
  ${CMAKE_SOURCE_DIR}/lib/libmqttcli.a
)
