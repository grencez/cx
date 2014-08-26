
# To see all commands:
#   make VERBOSE=1

# To use on windows, do:
#   cmake -G "NMake Makefiles"
#   nmake

cmake_minimum_required (VERSION 2.8)

project (cx)

set (CxPath ${CMAKE_CURRENT_SOURCE_DIR}/../cx)
set (CxPpPath ${CMAKE_CURRENT_SOURCE_DIR}/../cx-pp)
set (BinPath ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set (BldPath cx-new)

list (APPEND CFiles test.c)

include (${CxPath}/include.cmake)

addbinexe (test test.c)
set_target_properties (test PROPERTIES
  OUTPUT_NAME test
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target (pp
  DEPENDS cx
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CxBldPath} ${CxPpPath})



