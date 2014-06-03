#if !defined(AFX_SCXMLCONDICIONCOMPUESTA_H__BE536FAD_A223_4EF1_BE72_B4F11299828A__INCLUDED_)
#define AFX_SCXMLCONDICIONCOMPUESTA_H__BE536FAD_A223_4EF1_BE72_B4F11299828A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLCondicion.h"

#define SCXMLCOND_AND 0
#define SCXMLCOND_OR 1

class SCXMLCondicionCompuesta:public SCXMLCondicion	{
private:
	SCXMLCondicion *cond1;
	SCXMLCondicion *cond2;
	int operador;
public:
	SCXMLCondicionCompuesta(SCXMLCondicion *c1,SCXMLCondicion *c2,int tipo);
	SCXMLCondicionCompuesta(SCXMLCondicion *c1,SCXMLCondicion *c2,const char *cadena);
	virtual ~SCXMLCondicionCompuesta();
	virtual bool evaluar(SCXMLDataModel *d) const;
};
#endif