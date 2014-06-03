#if !defined(AFX_SCXMLEXPRESIONVAR_H__B9CD9402_6425_4A4F_8252_AE09B56AD2BA__INCLUDED_)
#define AFX_SCXMLEXPRESIONVAR_H__B9CD9402_6425_4A4F_8252_AE09B56AD2BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLExpresion.h"

class SCXMLExpresionVar:public SCXMLExpresion	{
private:
	char *lugar;
public:
	/* Métodos de evaluación. Si un dato de tipo cadena intenta ser convertido a número, se devolverá el valor 0.*/
	virtual char *evaluarCadena(SCXMLDataModel *datos) const;
	virtual int evaluarEntero(SCXMLDataModel *datos) const;
	virtual float evaluarFlotante(SCXMLDataModel *datos) const;
	/* Constructor y destructor. */
	SCXMLExpresionVar(char *l);
	virtual ~SCXMLExpresionVar();
};
#endif