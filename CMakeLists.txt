
# To see all commands:
#   make VERBOSE=1

# To use on windows, do:
#   cmake -G "NMake Makefiles"
#   nmake

# Just a guess, can probably go older.
cmake_minimum_required (VERSION 2.8)

project (Cx)
set (CxPpPath ../cx-pp)
set (EXECUTABLE_OUTPUT_PATH ".")

set (CSources
    bstree.c
    fileb.c
    ospc.c
    rbtree.c
    syscx.c
    )

set (GuiName gui)

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

add_executable (verify verif.c ${CSources})
set_target_properties (verify PROPERTIES
    OUTPUT_NAME verify)

include_directories (".")

