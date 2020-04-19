set(default_compile_commands_export ON)

if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
  message(STATUS "Setting compile commands export to '${default_compile_commands_export}' by default")
  set(CMAKE_EXPORT_COMPILE_COMMANDS "${default_compile_commands_export}" CACHE
    STRING "Export compile commands" FORCE)
endif()
