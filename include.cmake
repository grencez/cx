
set (PfxBldPath ${CMAKE_CURRENT_BINARY_DIR}/bld)
set (BldPath ${PfxBldPath}/${BldPath})

set (CxBldPath ${PfxBldPath}/cx)

if (DEFINED CxPpPath)
    set (CxPpExe ${CxPpPath}/cx)
endif ()

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
    set (CMAKE_C_FLAGS "-Wall -Wextra -Werror -ansi -pedantic")
else ()
    set (CMAKE_C_FLAGS "/W4 /MP")
    # Disable warning: 'fopen' unsafe, use fopen_s instead
    add_definitions ("-D_CRT_SECURE_NO_WARNINGS")
    # Other warnings disabled in def.h via #pragmas.

    set (CMAKE_BUILD_TYPE RELEASE)

endif ()


foreach (f ${CxCFiles})
    list (APPEND CxFullCFiles ${CxBldPath}/${f})
endforeach ()


if (DEFINED CxPpPath)
    add_executable (cx ${CxBldPath}/cx.c ${CxFullCFiles})
    set_target_properties (cx PROPERTIES
        OUTPUT_NAME cx
        RUNTIME_OUTPUT_DIRECTORY ${BinPath})

    list (APPEND CxPpSources ${CxPpPath}/cx.c)
    foreach (f ${CxCFiles})
        list (APPEND CxPpSources ${CxPpPath}/${f})
    endforeach ()
    add_executable (CxPpExe ${CxPpSources})
    set_target_properties (CxPpExe PROPERTIES
        OUTPUT_NAME cx
        RUNTIME_OUTPUT_DIRECTORY ${CxPpPath})
else ()
    add_executable (cx IMPORTED)
    set_property (TARGET cx
        PROPERTY IMPORTED_LOCATION ${BinPath}/cx)
endif ()


function (cx_source file)
    if (DEFINED CxPpPath)
        set (exe CxPpExe)
    else ()
        set (exe cx)
    endif ()
    add_custom_command (
        OUTPUT ${CxBldPath}/${file}
        COMMAND ${exe} -x ${CxPath}/${file} -o ${CxBldPath}/${file}
        DEPENDS ${CxPath}/${file})
    set_source_files_properties (${CxBldPath}/${file} PROPERTIES GENERATED TRUE)
endfunction ()

function (bld_source file)
    add_custom_command (
        OUTPUT ${BldPath}/${file}
        COMMAND cx -x ${CMAKE_CURRENT_SOURCE_DIR}/${file} -o ${BldPath}/${file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
    set_source_files_properties (${BldPath}/${file} PROPERTIES GENERATED TRUE)
endfunction ()

foreach (f ${CxHFiles})
    cx_source (${f})
    list (APPEND CxFullHFiles ${CxBldPath}/${f})
endforeach ()

if (DEFINED CxPpPath)
    cx_source (cx.c)
endif ()
foreach (f ${CxCFiles})
    cx_source (${f})
endforeach ()

foreach (f ${HFiles})
    bld_source (${f})
    list (APPEND FullHFiles ${BldPath}/${f})
endforeach ()

foreach (f ${CFiles})
    bld_source (${f})
    list (APPEND FullCFiles ${BldPath}/${f})
endforeach ()

set_property (SOURCE ${CxFullCFiles}
    APPEND PROPERTY OBJECT_DEPENDS ${CxFullHFiles})
if (DEFINED CxPpPath)
    set_property (SOURCE ${CxBldPath}/cx.c
        APPEND PROPERTY OBJECT_DEPENDS ${CxFullHFiles})
endif ()
set_property (SOURCE ${FullCFiles}
    APPEND PROPERTY OBJECT_DEPENDS ${CxFullHFiles} ${FullHFiles})


include_directories (${PfxBldPath})

file (MAKE_DIRECTORY ${PfxBldPath})
file (MAKE_DIRECTORY ${CxBldPath})
file (MAKE_DIRECTORY ${BldPath})


add_library (CxLib STATIC ${CxFullCFiles})
set_target_properties (CxLib PROPERTIES OUTPUT_NAME "cx")

function (addbinexe f)
    set (src_files ${BldPath}/${f}.c)
    foreach (src_file ${ARGN})
        list (APPEND src_files ${BldPath}/${src_file})
    endforeach ()

    add_executable (${f} ${src_files})
    target_link_libraries (${f} CxLib)
endfunction ()

