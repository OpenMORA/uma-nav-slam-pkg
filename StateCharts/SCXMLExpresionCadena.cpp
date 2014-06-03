#include "SCXMLExpresionCadena.h"

SCXMLExpresionCadena::~SCXMLExpresionCadena()	{
	free(valor);
}

char *SCXMLExpresionCadena::evaluarCadena(SCXMLDataModel *datos) const	{
	return _strdup(valor);
}

SCXMLExpresionCadena::SCXMLExpresionCadena(char *cadena)	{
	valor=_strdup(cadena);
}