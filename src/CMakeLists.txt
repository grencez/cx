
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
set (CxPpPath ${CMAKE_CURRENT_SOURCE_DIR}/../../cx-pp)
if (NOT DEFINED BinPath)
  set (BinPath ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
endif ()
set (BldPath cx-new)

list (APPEND CFiles test.c)
list (APPEND HFiles testlist.h testcxx.h)

list (APPEND CCFiles testcxx.cc)

include (${CxPath}/include.cmake)

add_executable (file2array file2array.c)
set_target_properties (file2array  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${BinPath})

addbinexe (test_exe test.c testcxx.cc)
set_target_properties (test_exe PROPERTIES
  OUTPUT_NAME test
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target (pp
  DEPENDS cx
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CxBldPath} ${CxPpPath})


enable_testing ()

cat_parenthesized (TestNames "${CMAKE_CURRENT_SOURCE_DIR}/testlist.h")

foreach (testname ${TestNames})
  add_test (NAME ${testname}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND test_exe ${testname})
endforeach ()

