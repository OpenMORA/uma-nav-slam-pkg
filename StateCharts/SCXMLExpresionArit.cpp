#include "SCXMLExpresionArit.h"
#include <math.h>
#include <sstream>

SCXMLExpresionArit::~SCXMLExpresionArit()	{
	delete expr1;
	delete expr2;
}

float SCXMLExpresionArit::evaluarFlotante(SCXMLDataModel *d) const	{
	if (tipo==SCXML_EXPRESION_MODULO) return (float)evaluarEntero(d);
	float f1=expr1->evaluarFlotante(d);
	float f2=expr2->evaluarFlotante(d);
	switch (tipo)	{
	case SCXML_EXPRESION_SUMA:return f1+f2;
	case SCXML_EXPRESION_RESTA:return f1-f2;
	case SCXML_EXPRESION_PRODUCTO:return f1*f2;
	case SCXML_EXPRESION_COCIENTE:return f1/f2;
	case SCXML_EXPRESION_POTENCIA:return pow(f1,f2);
	default:throw runtime_error("Operación desconocida.");
	}
}

int SCXMLExpresionArit::evaluarEntero(SCXMLDataModel *d) const	{
	if (tipo!=SCXML_EXPRESION_MODULO) return (int)evaluarFlotante(d);
	int i1=expr1->evaluarEntero(d);
	int i2=expr2->evaluarEntero(d);
	switch (tipo)	{
	case SCXML_EXPRESION_MODULO:return i1%i2;
	default:throw runtime_error("Operación desconocida.");
	}
}

char *SCXMLExpresionArit::evaluarCadena(SCXMLDataModel *d) const	{
	stringstream s;
	s<<evaluarFlotante(d);
	return _strdup(s.str().c_str());
}

SCXMLExpresionArit::SCXMLExpresionArit(SCXMLExpresion *e1,SCXMLExpresion *e2,int t)	{
	expr1=e1;
	expr2=e2;
	tipo=t;
}

SCXMLExpresionArit::SCXMLExpresionArit(SCXMLExpresion *e1,SCXMLExpresion *e2,char c)	{
	expr1=e1;
	expr2=e2;
	switch (c)	{
	case '+':tipo=SCXML_EXPRESION_SUMA;break;
	case '-':tipo=SCXML_EXPRESION_RESTA;break;
	case '*':tipo=SCXML_EXPRESION_PRODUCTO;break;
	case '/':tipo=SCXML_EXPRESION_COCIENTE;break;
	case '%':tipo=SCXML_EXPRESION_MODULO;break;
	case '^':tipo=SCXML_EXPRESION_POTENCIA;break;
	default:throw runtime_error("Operación desconocida.");
	}
}
