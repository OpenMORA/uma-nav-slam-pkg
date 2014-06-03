#include "SCXMLExpresionMOOSDBVble.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <mrpt/system/datetime.h> // for now()

SCXMLExpresionMOOSDBVble::~SCXMLExpresionMOOSDBVble()	{}

char *SCXMLExpresionMOOSDBVble::evaluarCadena(SCXMLDataModel *d) const	{
	stringstream ss;
	double val=d->rut->check(vble_name.c_str());
//	printf("Ahora iria a MOOSDB y miraría la vble...y vale %f\n",val);
	ss<<val;
	return strdup(ss.str().c_str());
}

int SCXMLExpresionMOOSDBVble::evaluarEntero(SCXMLDataModel *d) const	{
	return (int)d->rut->check(vble_name.c_str());
}

float SCXMLExpresionMOOSDBVble::evaluarFlotante(SCXMLDataModel *d) const	{
	return (float)d->rut->check(vble_name.c_str());
}

SCXMLExpresionMOOSDBVble::SCXMLExpresionMOOSDBVble(char* name)	{
	//srand( static_cast<unsigned int>( mrpt::system::now() ) );
	
	vble_name=name;
	//printf("Inside %s\n",vble_name.c_str());
}
