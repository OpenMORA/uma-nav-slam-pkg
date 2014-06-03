#if !defined(AFX_SCXMLESTADOSIMPLE_H__6D5234FF_4EA2_44BB_A9C1_5465B00793E8__INCLUDED_)
#define AFX_SCXMLESTADOSIMPLE_H__6D5234FF_4EA2_44BB_A9C1_5465B00793E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEstado.h"

class SCXMLEstadoSimple:public SCXMLEstado	{
public:
	SCXMLEstadoSimple(const NodoXML *n,SCXMLEstado *p,SCXMLDataModel *d,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r);
	virtual ~SCXMLEstadoSimple();
	virtual bool soyYo(const char *estados) const;
	virtual bool corriendo(const char *estado) const;
	virtual bool esFinal() const;
	virtual void haTerminado(const char *);
	virtual void entrar(const char *hijo);
	virtual bool transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *d);
	virtual list<TransicionActiva *> *evento(const char *n) const;
};
#endif