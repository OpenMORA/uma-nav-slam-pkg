#if !defined(AFX_SCXMLEVENT_H__78E9629E_F478_4067_9764_C52DD2648C48__INCLUDED_)
#define AFX_SCXMLEVENT_H__78E9629E_F478_4067_9764_C52DD2648C48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEjecutable.h"
#include <list>

class SCXMLEvent:public SCXMLEjecutable	{
private:
	char *nombre;
	list<const char *> *colaInterna;
public:
	SCXMLEvent(const char *n,list<const char *> *l);
	virtual ~SCXMLEvent();
	virtual void ejecutar(SCXMLDataModel *d) const;
};

#endif // !defined(AFX_SCXMLEVENT_H__78E9629E_F478_4067_9764_C52DD2648C48__INCLUDED_)
