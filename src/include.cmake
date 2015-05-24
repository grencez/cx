
set (PfxBldPath ${CMAKE_CURRENT_BINARY_DIR}/bld)
set (BldPath ${PfxBldPath}/${BldPath})

set (CxBldPath ${PfxBldPath}/cx)

if (DEFINED CxPpPath)
  set (CxPpExe ${CxPpPath}/cx)
endif ()

#set (CMAKE_CXX_COMPILER g++)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set (CxDeps alphatab bstree
  xfile ofile fileb
  ospc rbtree
  sesp sxpn syscx urandom)

list (APPEND CxHFiles
	associa.h
	bittable.h
	def.h
	gmrand.h
	lgtable.h
	synhax.h
	table.h
  )

list (APPEND CxHHFiles
  alphatab.hh map.hh synhax.hh set.hh
  table.hh bittable.hh lgtable.hh
  urandom.hh
  xfile.hh ofile.hh fileb.hh
  mpidissem.hh kautz.hh
  )

list (APPEND CxCCFiles
  mpidissem.cc kautz.cc
  )

foreach (d ${CxDeps})
  list (APPEND CxCFiles ${d}.c)
  list (APPEND CxHFiles ${d}.h)
endforeach ()

if (UNIX)
  foreach (flags_vbl
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
      CMAKE_C_FLAGS_RELWITHDEBINFO)
    string (REGEX REPLACE "(^| )[/-]D *NDEBUG($| )" " "
      "${flags_vbl}" "${${flags_vbl}}")
  endforeach ()

  if (NOT CMAKE_BUILD_TYPE)
    #set (CMAKE_BUILD_TYPE DEBUG)
    set (CMAKE_BUILD_TYPE RelWithDebInfo)
  else ()
  endif ()
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  set (DEFAULT_COMPILE_FLAGS "-ansi -pedantic")
  #set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
else ()
  set (CMAKE_C_FLAGS "/W4 /MP")
  set (DEFAULT_COMPILE_FLAGS "")
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
    RUNTIME_OUTPUT_DIRECTORY ${BinPath}
    COMPILE_FLAGS ${DEFAULT_COMPILE_FLAGS})

  list (APPEND CxPpSources ${CxPpPath}/cx.c)
  foreach (f ${CxCFiles})
    list (APPEND CxPpSources ${CxPpPath}/${f})
  endforeach ()
  add_executable (CxPpExe ${CxPpSources})
  set_target_properties (CxPpExe PROPERTIES
    OUTPUT_NAME cx
    RUNTIME_OUTPUT_DIRECTORY ${CxPpPath}
    COMPILE_FLAGS ${DEFAULT_COMPILE_FLAGS})
else ()
  add_executable (cx IMPORTED)
  set_property (TARGET cx
    PROPERTY IMPORTED_LOCATION ${BinPath}/cx)
endif ()

function (set_bld_cfile_properties file)
  #set (nice_filepath)
  #string(REPLACE "${PfxBldPath}/" "" nice_filepath "${file}")
  set_source_files_properties (${file} PROPERTIES
    GENERATED TRUE)
endfunction ()

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
  set_bld_cfile_properties (${CxBldPath}/${file})
endfunction ()

function (cx_cxx_source file)
  add_custom_command (
    OUTPUT ${CxBldPath}/${file}
    COMMAND ${CMAKE_COMMAND} -E copy ${CxPath}/${file} ${CxBldPath}/${file}
    DEPENDS ${CxPath}/${file})
  set_bld_cfile_properties (${CxBldPath}/${file})
endfunction ()

function (bld_source file)
  add_custom_command (
    OUTPUT ${BldPath}/${file}
    COMMAND cx -x ${CMAKE_CURRENT_SOURCE_DIR}/${file} -o ${BldPath}/${file}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  set_bld_cfile_properties (${BldPath}/${file})
endfunction ()

function (bld_cxx_source file)
  add_custom_command (
    OUTPUT ${BldPath}/${file}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${file} ${BldPath}/${file}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  set_bld_cfile_properties (${BldPath}/${file})
endfunction ()

foreach (f ${CxHFiles})
  cx_source (${f})
  list (APPEND CxFullHFiles ${CxBldPath}/${f})
endforeach ()

foreach (f ${CxHHFiles})
  cx_cxx_source (${f})
  list (APPEND CxFullHHFiles ${CxBldPath}/${f})
endforeach ()

foreach (f ${CxCCFiles})
  cx_cxx_source (${f})
  list (APPEND CxFullCCFiles ${CxBldPath}/${f})
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

foreach (f ${HHFiles})
  bld_cxx_source (${f})
  list (APPEND FullHHFiles ${BldPath}/${f})
endforeach ()

foreach (f ${CCFiles})
  bld_cxx_source (${f})
  list (APPEND FullCCFiles ${BldPath}/${f})
endforeach ()

if (DEFINED CxPpPath)
  set_property (SOURCE ${CxBldPath}/cx.c
    APPEND PROPERTY OBJECT_DEPENDS ${CxFullHFiles})
endif ()


include_directories (${PfxBldPath})

file (MAKE_DIRECTORY ${PfxBldPath})
file (MAKE_DIRECTORY ${CxBldPath})
file (MAKE_DIRECTORY ${BldPath})

add_custom_target (GenSources SOURCES ${FullCFiles} ${FullHFiles} ${FullCCFiles} ${FullHHFiles}
  ${CxFullHFiles} ${CxFullCCFiles} ${CxFullHHFiles})

add_library (CxLib STATIC ${CxFullCFiles})
set_target_properties (CxLib PROPERTIES OUTPUT_NAME "cx")
set_target_properties (CxLib PROPERTIES COMPILE_FLAGS ${DEFAULT_COMPILE_FLAGS})
add_dependencies(CxLib GenSources)

function (cat_parenthesized dst f)
  file(READ ${f} tmp)
  string(REPLACE "\n" ";" tmp "${tmp}")
  string(REGEX REPLACE "[^;\\(]*\\( *([^\\) ]*) *\\) *" "\\1" tmp "${tmp}")
  set(${dst} ${tmp} PARENT_SCOPE)
endfunction ()

function (pfxeq dst pfx s)
  string(LENGTH ${pfx} len_pfx)
  string(LENGTH ${s} len_s)
  if (len_pfx GREATER len_s)
    set(${dst} FALSE PARENT_SCOPE)
  else ()
    string(SUBSTRING ${s} 0 ${len_pfx} s)
    if (pfx STREQUAL s)
      set(${dst} TRUE PARENT_SCOPE)
    else ()
      set(${dst} FALSE PARENT_SCOPE)
    endif ()
  endif ()
endfunction ()

function (addbinexe f)
  set (src_files)
  foreach (src_file ${ARGN})
    pfxeq(path_is_set ${PfxBldPath} ${src_file})
    if (path_is_set)
      list (APPEND src_files ${src_file})
    else ()
      list (APPEND src_files ${BldPath}/${src_file})
    endif ()
  endforeach ()

  add_executable (${f} ${src_files})
  add_dependencies(${f} GenSources)
  set_target_properties (${f} PROPERTIES
    COMPILE_FLAGS ${DEFAULT_COMPILE_FLAGS})
  target_link_libraries (${f} CxLib)
endfunction ()

