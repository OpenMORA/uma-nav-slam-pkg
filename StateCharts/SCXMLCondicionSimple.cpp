#include "SCXMLCondicionSimple.h"

SCXMLCondicionSimple::~SCXMLCondicionSimple()	{
	delete expr1;
	delete expr2;
}

bool SCXMLCondicionSimple::evaluar(SCXMLDataModel *d) const	{
	float f1=expr1->evaluarFlotante(d);
	float f2=expr2->evaluarFlotante(d);
	switch (operador)	{
	case SCXMLCOND_IGUAL:return f1==f2;
	case SCXMLCOND_DISTINTO:return f1!=f2;
	case SCXMLCOND_MENOR:return f1<f2;
	case SCXMLCOND_MENORIGUAL:return f1<=f2;
	case SCXMLCOND_MAYOR:return f1>f2;
	case SCXMLCOND_MAYORIGUAL:return f1>=f2;
	default:throw runtime_error("Operador desconocido.");
	}
}

SCXMLCondicionSimple::SCXMLCondicionSimple(SCXMLExpresion *e1,SCXMLExpresion *e2,int tipo)	{
	expr1=e1;
	expr2=e2;
	operador=tipo;
}

SCXMLCondicionSimple::SCXMLCondicionSimple(SCXMLExpresion *e1,SCXMLExpresion *e2,const char *cadena)	{
	expr1=e1;
	expr2=e2;
	if (!strcmp("==",cadena)) operador=SCXMLCOND_IGUAL;
	else if (!strcmp("!=",cadena)||!strcmp("<>",cadena)) operador=SCXMLCOND_DISTINTO;
	else if (!strcmp("<",cadena)) operador=SCXMLCOND_MENOR;
	else if (!strcmp("<=",cadena)||!strcmp("=<",cadena)) operador=SCXMLCOND_MENORIGUAL;
	else if (!strcmp(">",cadena)) operador=SCXMLCOND_MAYOR;
	else if (!strcmp(">=",cadena)||!strcmp("<=",cadena)) operador=SCXMLCOND_MAYORIGUAL;
	else throw runtime_error("Operador desconocido.");
}
