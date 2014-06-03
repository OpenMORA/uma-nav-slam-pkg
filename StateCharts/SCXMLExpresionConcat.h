#if !defined(AFX_SCXMLEXPRESIONCONCAT_H__6781C6EA_7F38_4907_8E89_491D085826FD__INCLUDED_)
#define AFX_SCXMLEXPRESIONCONCAT_H__6781C6EA_7F38_4907_8E89_491D085826FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLExpresion.h"

class SCXMLExpresionConcat:public SCXMLExpresion	{
private:
	SCXMLExpresion *expr1;
	SCXMLExpresion *expr2;
public:
	/* Constructor. */
	SCXMLExpresionConcat(SCXMLExpresion *e1,SCXMLExpresion *e2);
	/* Método de evaluación de cadena. Los otros métodos se heredan de SCXMLExpresion. */
	virtual char *evaluarCadena(SCXMLDataModel *d) const;
	/* Destructor. */
	virtual ~SCXMLExpresionConcat();
};
#endif