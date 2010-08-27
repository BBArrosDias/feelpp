#
# Find the PARMETIS includes and libraries
#
# ParMETIS is an MPI-based parallel library that implements a variety of algorithms for
# partitioning unstructured graphs, meshes, and for computing fill-reducing orderings of
# sparse matrices. It can be found at:
# 	http://www-users.cs.umn.edu/~karypis/metis/parmetis/index.html
#
# PARMETIS_INCLUDE_DIR - where to find autopack.h
# PARMETIS_LIBRARIES   - List of fully qualified libraries to link against.
# PARMETIS_FOUND       - Do not attempt to use if "no" or undefined.

FIND_PATH(METIS_INCLUDE_DIR metis.h
  /usr/local/include
  /usr/include
  /usr/include/metis
  )
message( STATUS ${METIS_INCLUDE_DIR} )
#FIND_LIBRARY(PARMETIS_LIBRARY parmetis
#  /usr/local/lib
#  /usr/lib
#  )

FIND_LIBRARY(METIS_LIBRARY metis
  /usr/local/lib
  /usr/lib
  )

IF(METIS_INCLUDE_DIR)
  ADD_DEFINITIONS( -DHAVE_METIS_H=1 )

  IF(METIS_LIBRARY)
    SET( METIS_LIBRARIES ${PARMETIS_LIBRARY} ${METIS_LIBRARY})
    SET( METIS_FOUND "YES" )
  ENDIF(METIS_LIBRARY)
ENDIF(METIS_INCLUDE_DIR)

MARK_AS_ADVANCED( METIS_INCLUDE_DIR METIS_LIBRARY )
