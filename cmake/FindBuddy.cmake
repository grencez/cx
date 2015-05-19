FIND_PATH(BUDDY_INCLUDE_DIR NAMES bdd.h)

FIND_LIBRARY(
  BUDDY_LIBRARY
  NAMES bdd)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Buddy
  DEFAULT_MSG
  BUDDY_LIBRARY BUDDY_INCLUDE_DIR)

if(BUDDY_FOUND)
  set(BUDDY_LIBRARIES ${BUDDY_LIBRARY})
else ()
  set(BUDDY_LIBRARIES)
endif ()

mark_as_advanced(
  BUDDY_INCLUDE_DIR
  BUDDY_LIBRARY
  )
