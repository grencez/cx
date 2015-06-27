
# To see all commands:
#   make VERBOSE=1

# To use on windows, do:
#   cmake -G "NMake Makefiles"
#   nmake

#SET(CMAKE_CXX_FLAGS_DEBUG "-g3")
#SET(CMAKE_C_FLAGS_DEBUG "-g3")
#set (CMAKE_BUILD_TYPE DEBUG)
#set (CMAKE_BUILD_TYPE RELEASE)
#set (CMAKE_BUILD_TYPE RelWithDebInfo)

cmake_minimum_required (VERSION 2.8)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project (cx)

set (CxPath ${CMAKE_CURRENT_SOURCE_DIR})
set (CxTopPath ${CxPath}/..)

set (CxPpPath ${CxTopPath}/../cx-pp)
if (NOT DEFINED BinPath)
  set (BinPath ${CxTopPath}/bin)
endif ()
set (BldPath cx-new)

list (APPEND CFiles test.c)
list (APPEND HFiles testcxx.h)

list (APPEND CCFiles testcxx.cc)

include (${CxPath}/include.cmake)

add_executable (cembed cembed.c)
set_target_properties (cembed PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${BinPath})

add_executable (cswitch cswitch.c)
set_target_properties (cswitch PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${BinPath})


file (MAKE_DIRECTORY ${BldPath}/test-dep)
add_custom_command (
  OUTPUT ${BldPath}/test-dep/switch.c
  COMMAND cswitch testname
  -case-pfx "fn = testfn_"
  -array AllTests
  -x ${CxPath}/testlist.txt
  -o ${BldPath}/test-dep/switch.c
  DEPENDS cswitch ${CxPath}/testlist.txt)
add_custom_target (TestSwitchSources SOURCES ${BldPath}/test-dep/switch.c)


addbinexe (test_exe test.c testcxx.cc)
set_target_properties (test_exe PROPERTIES
  OUTPUT_NAME test
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(test_exe TestSwitchSources)

add_custom_target (pp
  DEPENDS cx
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CxBldPath} ${CxPpPath})


enable_testing ()

file(READ ${CxPath}/testlist.txt TestNames)
string(REPLACE "\n" ";" TestNames "${TestNames}")

foreach (testname ${TestNames})
  add_test (NAME ${testname}
    WORKING_DIRECTORY ${CxPath}
    COMMAND test_exe ${testname})
endforeach ()

