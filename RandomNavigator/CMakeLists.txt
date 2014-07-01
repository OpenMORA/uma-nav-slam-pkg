PROJECT(RandomNavigator)

# Use MRPT:
FIND_PACKAGE( MRPT REQUIRED gui hmtslam ) # hwdrivers ... any other sub-library

INCLUDE_DIRECTORIES( ${MOOS_INCLUDE_DIRS}  )
LINK_DIRECTORIES( ${MOOS_LIBRARY_DIRS} )

# Add executable
ADD_EXECUTABLE( ${PROJECT_NAME}
	main.cpp
	CRandomNavigatorApp.h 
	CRandomNavigatorApp.cpp	
	)

# Link the executable
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MOOS_LIBRARIES} ${MRPT_LIBS} )
