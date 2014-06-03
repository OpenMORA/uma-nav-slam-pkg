#include "SCXMLExpresionVar.h"

SCXMLExpresionVar::~SCXMLExpresionVar()	{
	free(lugar);
}

char *SCXMLExpresionVar::evaluarCadena(SCXMLDataModel *datos) const	{
	SCXMLData *d=datos->obtenerValor(lugar);
	if (!d) return NULL;
	if (d->esArbol) return NULL;
	else return _strdup(d->datos.cadena);
}

int SCXMLExpresionVar::evaluarEntero(SCXMLDataModel *datos) const	{
	char *cad=evaluarCadena(datos);
	if (!cad) return 0;
	else	{
		int res=atoi(cad);
		free(cad);
		return res;
	}
}

float SCXMLExpresionVar::evaluarFlotante(SCXMLDataModel *datos) const	{
	char *cad=evaluarCadena(datos);
	if (!cad) return 0;
	else	{
		float res=atof(cad);
		free(cad);
		return res;
	}
}

SCXMLExpresionVar::SCXMLExpresionVar(char *l)	{
	lugar=_strdup(l);
}