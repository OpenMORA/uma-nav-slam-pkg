#if !defined(AFX_SCXMLCONDICION_H__3155CD60_285C_43E7_8037_D86DBFA9C732__INCLUDED_)
#define AFX_SCXMLCONDICION_H__3155CD60_285C_43E7_8037_D86DBFA9C732__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXML.h"
#include "SCXMLExpresion.h"

class SCXMLCondicion	{
public:
	virtual bool evaluar(SCXMLDataModel *d) const=0;
	static SCXMLCondicion *crearCondicion(const char *cadena);
};

#endif