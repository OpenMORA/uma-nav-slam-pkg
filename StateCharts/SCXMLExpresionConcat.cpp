#include "SCXMLExpresionConcat.h"

SCXMLExpresionConcat::~SCXMLExpresionConcat()	{
	delete expr1;
	delete expr2;
}

char *SCXMLExpresionConcat::evaluarCadena(SCXMLDataModel *d) const	{
	char *s1=expr1->evaluarCadena(d);
	char *s2=expr2->evaluarCadena(d);
	char *res=new char[strlen(s1)+strlen(s2)+1];
	strcpy(res,s1);
	strcat(res,s2);
	free(s1);
	free(s2);
	return res;
}

SCXMLExpresionConcat::SCXMLExpresionConcat(SCXMLExpresion *e1,SCXMLExpresion *e2)	{
	expr1=e1;
	expr2=e2;
}
