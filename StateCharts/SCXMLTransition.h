#if !defined(AFX_SCXMLTRANSITION_H__EA65CDF5_DE3F_4E37_9497_7DC4DFD091F8__INCLUDED_)
#define AFX_SCXMLTRANSITION_H__EA65CDF5_DE3F_4E37_9497_7DC4DFD091F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXML.h"
#include "SCXMLDataModel.h"
#include "SCXMLCondicion.h"
#include "SCXMLEjecutable.h"

class SCXMLTransition	{
private:
	SCXMLDataModel *modelo;
	char *event;
	SCXMLCondicion *cond;
	char *target;
	SCXMLEjecutable **sentencias;
public:
	bool activar() const;	//true si la transición se activa (aunque no haya evento), false si no.
	bool activar(const char *evento) const;	//true si la transición se activa (hay que fijarse también en el datamodel)
	void ejecutar(SCXMLData *eventData);
	SCXMLTransition(const NodoXML *n,SCXMLDataModel *d,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r);
	virtual ~SCXMLTransition();
	const char *getTarget() const;
};
#endif