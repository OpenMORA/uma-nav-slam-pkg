#include "SCXMLExpresionRandom.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <mrpt/system/datetime.h> // for now()

SCXMLExpresionRandom::~SCXMLExpresionRandom()	{}

inline float getValor()	{
	return ((float)rand())/((float)RAND_MAX);
}

char *SCXMLExpresionRandom::evaluarCadena(SCXMLDataModel *d) const	{
	stringstream ss;
	ss<<getValor();
	return strdup(ss.str().c_str());
}

int SCXMLExpresionRandom::evaluarEntero(SCXMLDataModel *d) const	{
	return (int)getValor();
}

float SCXMLExpresionRandom::evaluarFlotante(SCXMLDataModel *d) const	{
	return getValor();
}

SCXMLExpresionRandom::SCXMLExpresionRandom()	{
	srand( static_cast<unsigned int>( mrpt::system::now() ) );
}
