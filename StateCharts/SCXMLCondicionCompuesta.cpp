#include "SCXMLCondicionCompuesta.h"

SCXMLCondicionCompuesta::~SCXMLCondicionCompuesta()	{
	delete cond1;
	delete cond2;
}

bool SCXMLCondicionCompuesta::evaluar(SCXMLDataModel *d) const	{
	bool b1=cond1->evaluar(d);
	bool b2=cond2->evaluar(d);
	switch (operador)	{
	case SCXMLCOND_AND:return b1&&b2;
	case SCXMLCOND_OR:return b1||b2;
	default:throw runtime_error("Operador desconocido.");
	}
}

SCXMLCondicionCompuesta::SCXMLCondicionCompuesta(SCXMLCondicion *c1,SCXMLCondicion *c2,int tipo)	{
	cond1=c1;
	cond2=c2;
	operador=tipo;
}

SCXMLCondicionCompuesta::SCXMLCondicionCompuesta(SCXMLCondicion *c1,SCXMLCondicion *c2,const char *cadena)	{
	cond1=c1;
	cond2=c2;
	if (!strcmp("&&",cadena)) operador=SCXMLCOND_AND;
	else if (!strcmp("||",cadena)) operador=SCXMLCOND_OR;
	else throw runtime_error("Operador desconocido.");
}
