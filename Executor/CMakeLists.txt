PROJECT(Executor)

# Use MRPT:
FIND_PACKAGE( MRPT REQUIRED slam  ) # hmtslam hwdrivers ... any other sub-library

INCLUDE_DIRECTORIES( ${MOOS_INCLUDE_DIRS}  )
LINK_DIRECTORIES( ${MOOS_LIBRARY_DIRS} )

# Add executable
ADD_EXECUTABLE( ${PROJECT_NAME}
	main.cpp
	TExecutor.cpp TExecutor.hpp CExecutorApp.hpp CExecutorApp.cpp
	)

# Link the executable
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MOOS_LIBRARIES} ${MRPT_LIBS})
IF(WIN32)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME}  Winmm.lib)
ENDIF(WIN32)

