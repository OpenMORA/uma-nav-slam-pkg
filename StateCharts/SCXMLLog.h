#if !defined(AFX_SCXMLLOG_H__CF6930BC_E082_4F4E_93BD_EDA3B511C856__INCLUDED_)
#define AFX_SCXMLLOG_H__CF6930BC_E082_4F4E_93BD_EDA3B511C856__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEjecutable.h"
#include "SCXMLExpresion.h"

class SCXMLLog:public SCXMLEjecutable	{
private:
	Rutinas *rut;
	SCXMLExpresion *expr;
	char *label;
public:
	SCXMLLog(SCXMLExpresion *e,const char *lab,Rutinas *r);
	virtual ~SCXMLLog();
	virtual void ejecutar(SCXMLDataModel *d) const;
};

#endif