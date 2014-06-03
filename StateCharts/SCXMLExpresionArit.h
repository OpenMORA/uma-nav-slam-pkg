#if !defined(AFX_SCXMLEXPRESIONARIT_H__AD526F34_CE25_488D_A756_A73AAAECD5D9__INCLUDED_)
#define AFX_SCXMLEXPRESIONARIT_H__AD526F34_CE25_488D_A756_A73AAAECD5D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLExpresion.h"
#define SCXML_EXPRESION_SUMA 0
#define SCXML_EXPRESION_RESTA 1
#define SCXML_EXPRESION_PRODUCTO 2
#define SCXML_EXPRESION_COCIENTE 3
#define SCXML_EXPRESION_MODULO 4
#define SCXML_EXPRESION_POTENCIA 5

class SCXMLExpresionArit:public SCXMLExpresion	{
private:
	int tipo;
	SCXMLExpresion *expr1;
	SCXMLExpresion *expr2;
public:
	/* Constructores. Reciben las dos expresiones del operador y el tipo de operación. */
	SCXMLExpresionArit(SCXMLExpresion *e1,SCXMLExpresion *e2,int t);
	SCXMLExpresionArit(SCXMLExpresion *e1,SCXMLExpresion *e2,char c);
	/* Métodos de evaluación. */
	virtual char *evaluarCadena(SCXMLDataModel *d) const;
	virtual int evaluarEntero(SCXMLDataModel *d) const;
	virtual float evaluarFlotante(SCXMLDataModel *d) const;
	/* Destructor. */
	virtual ~SCXMLExpresionArit();
};

#endif