
# To see all commands:
#   make VERBOSE=1

# To use on windows, do:
#   cmake -G "NMake Makefiles"
#   nmake

cmake_minimum_required (VERSION 2.8)

project (cx)
set (CxPpPath ../cx-pp)
set (BinPath ../bin)

set (CxDeps bstree fileb ospc rbtree sxpn syscx)

list (APPEND CxHFiles
    associa.h
    alphatab.h
    bittable.h
    def.h
    lgtable.h
    synhax.h
    table.h
    )

foreach (d ${CxDeps})
    list (APPEND CxCFiles ${d}.c)
    list (APPEND CxHFiles ${d}.h)
endforeach ()

if (UNIX)
    if (NOT CMAKE_BUILD_TYPE)
        set (CMAKE_BUILD_TYPE DEBUG)
    else ()
    endif ()
    set (CMAKE_C_FLAGS "-Wall -Wextra -ansi -pedantic")
else ()
    set (CMAKE_C_FLAGS "/W4 /MP")
    # Disable warning: 'fopen' unsafe, use fopen_s instead
    add_definitions ("-D_CRT_SECURE_NO_WARNINGS")
    # Other warnings disabled in def.h via #pragmas.

    set (CMAKE_BUILD_TYPE RELEASE)
endif ()

list (APPEND CxPpSources ${CxPpPath}/cx.c)
foreach (f ${CxCFiles})
    list (APPEND CxPpSources ${CxPpPath}/${f})
endforeach ()

add_executable (CxPpExe ${CxPpSources})

set_target_properties (CxPpExe PROPERTIES
    OUTPUT_NAME cx
    RUNTIME_OUTPUT_DIRECTORY ${CxPpPath})

set (PfxBldPath ${CMAKE_CURRENT_SOURCE_DIR}/../cx-bld)
set (CxBldPath ${PfxBldPath}/cx)
set (BldPath ${PfxBldPath}/cx-new)

file(MAKE_DIRECTORY ${PfxBldPath})
file(MAKE_DIRECTORY ${CxBldPath})
file(MAKE_DIRECTORY ${BldPath})

function(cx_source file)
    add_custom_command (
        OUTPUT ${CxBldPath}/${file}
        COMMAND CxPpExe -x ${CMAKE_CURRENT_SOURCE_DIR}/${file} -o ${CxBldPath}/${file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endfunction()

function(bld_source file)
    add_custom_command (
        OUTPUT ${BldPath}/${file}
        COMMAND CxPpExe -x ${CMAKE_CURRENT_SOURCE_DIR}/${file} -o ${BldPath}/${file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
endfunction()

cx_source (cx.c)
foreach (f ${CxCFiles})
    cx_source (${f})
    list (APPEND CxSources ${CxBldPath}/${f})
endforeach ()
foreach (f ${CxHFiles})
    cx_source (${f})
    list (APPEND CxSources ${CxBldPath}/${f})
endforeach ()

bld_source (test.c)

add_executable (CxExe ${CxBldPath}/cx.c ${CxSources})
set_target_properties (CxExe PROPERTIES
    OUTPUT_NAME cx
    RUNTIME_OUTPUT_DIRECTORY ${BinPath})

add_executable (test ${BldPath}/test.c ${CxSources})
set_target_properties (test PROPERTIES
    OUTPUT_NAME test)

include_directories (${PfxBldPath})

