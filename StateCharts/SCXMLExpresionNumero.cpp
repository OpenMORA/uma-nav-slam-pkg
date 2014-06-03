#include "SCXMLExpresionNumero.h"
#include <sstream>

SCXMLExpresionNumero::~SCXMLExpresionNumero()	{}

char *SCXMLExpresionNumero::evaluarCadena(SCXMLDataModel *d) const	{
	stringstream ss;
	ss<<valor;
	return _strdup(ss.str().c_str());
}

int SCXMLExpresionNumero::evaluarEntero(SCXMLDataModel *d) const	{
	return (int)valor;
}

float SCXMLExpresionNumero::evaluarFlotante(SCXMLDataModel *d) const	{
	return valor;
}

SCXMLExpresionNumero::SCXMLExpresionNumero(float num)	{
	valor=num;
}
