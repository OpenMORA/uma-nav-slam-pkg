#if !defined(AFX_SCXMLIF_H__6CE606C5_348A_412B_A03A_E1BEABF27BD2__INCLUDED_)
#define AFX_SCXMLIF_H__6CE606C5_348A_412B_A03A_E1BEABF27BD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEjecutable.h"
#include "SCXMLCondicion.h"

typedef struct	{
	SCXMLCondicion *cond;
	SCXMLEjecutable **sentencias;
}	SCXMLBloqueIf;

class SCXMLIf:public SCXMLEjecutable	{
private:
	SCXMLBloqueIf **bloques;
public:
	SCXMLIf(const NodoXML *n,list<const char *> *l,list<EventoExterno *> *lE,Rutinas *r);
	virtual ~SCXMLIf();
	virtual void ejecutar(SCXMLDataModel *d) const;
};
#endif