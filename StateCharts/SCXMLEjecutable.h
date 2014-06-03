#if !defined(AFX_SCXMLEJECUTABLE_H__831249A4_96FD_4281_B8A1_60EDD749EE9E__INCLUDED_)
#define AFX_SCXMLEJECUTABLE_H__831249A4_96FD_4281_B8A1_60EDD749EE9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SCXMLDataModel.h"
#include <list>

#include <mrpt/system.h> // TTimeStamp

using namespace std;

typedef struct	{
	const char *nombre;
	const char *target;
	mrpt::system::TTimeStamp tstamp;
	SCXMLData *_eventData;
	unsigned int procesados;
}	EventoExterno;

class SCXMLEjecutable	{
public:
	/* Único método de esta clase abstracta. */
	virtual void ejecutar(SCXMLDataModel *d) const=0;
	static SCXMLEjecutable *crearEjecutable(const NodoXML *n,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r);
};
#endif
