# File: CMakeListst.txt for module pStateChart
PROJECT(StateChart)

# Use MRPT:
FIND_PACKAGE( MRPT REQUIRED slam gui  ) # hmtslam hwdrivers ... any other sub-library

# Defines used by embedded "expat" library
CHECK_FUNCTION_EXISTS(memmove HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(bcopy HAVE_BCOPY)
IF(HAVE_MEMMOVE)
	ADD_DEFINITIONS(/DHAVE_MEMMOVE)
ENDIF(HAVE_MEMMOVE)

# if we have a system expat, use it:
find_library(EXPAT_LIB expat)
CHECK_INCLUDE_FILE(expat.h HAS_EXPAT_HEADER)
if (EXPAT_LIB AND HAS_EXPAT_HEADER)
	#MESSAGE(STATUS "pStateChart: Using system expat lib")
	SET(EXPAT_SYSTEM_LIB 1)
else(EXPAT_LIB AND HAS_EXPAT_HEADER)
	#MESSAGE(STATUS "pStateChart: Using embedded expat lib")
	SET(EXPAT_SYSTEM_LIB 0)
	SET(EXPAT_SRCS
		Expat/xmlparse.c Expat/xmlrole.c Expat/xmltok.c
		)
endif (EXPAT_LIB AND HAS_EXPAT_HEADER)

INCLUDE_DIRECTORIES( ${MOOS_INCLUDE_DIRS}  )
LINK_DIRECTORIES( ${MOOS_LIBRARY_DIRS} )

# Add executable
ADD_EXECUTABLE( ${PROJECT_NAME}
	main.cpp CStateChartApp.hpp CStateChartApp.cpp
	ManejadorSCXML.cpp SCXML.cpp SCXMLAssign.cpp SCXMLCondicion.cpp SCXMLCondicionCompuesta.cpp
	 SCXMLCondicionSimple.cpp SCXMLDataModel.cpp SCXMLEjecutable.cpp SCXMLEstado.cpp
	 SCXMLEstadoCompuesto.cpp SCXMLEstadoFinal.cpp SCXMLEstadoParalelo.cpp SCXMLEstadoSimple.cpp
	 SCXMLEvent.cpp SCXMLExpresion.cpp SCXMLExpresionArit.cpp SCXMLExpresionCadena.cpp
	 SCXMLExpresionConcat.cpp SCXMLExpresionNumero.cpp SCXMLExpresionVar.cpp SCXMLIf.cpp
	 SCXMLLog.cpp SCXMLMaquinaDeEstados.cpp SCXMLSend.cpp SCXMLTransition.cpp SCXMLExpresionRandom.cpp
	 SCXMLExpresionMOOSDBVble.cpp

	TypedefsSCXML.h ManejadorSCXML.h

	${EXPAT_SRCS}
	)

# Link the executable
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MOOS_LIBRARIES} ${MRPT_LIBS} )
if(EXPAT_SYSTEM_LIB)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${EXPAT_LIB} )
endif(EXPAT_SYSTEM_LIB)

