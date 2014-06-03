#if !defined(AFX_SCXMLESTADOCOMPUESTO_H__46270EF7_4885_44CB_BFBE_D6A503FA9915__INCLUDED_)
#define AFX_SCXMLESTADOCOMPUESTO_H__46270EF7_4885_44CB_BFBE_D6A503FA9915__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEstado.h"

class SCXMLEstadoCompuesto:public SCXMLEstado	{
private:
	SCXMLEstado **hijos;
	const char *actual;
	char *inicial;
	char *nombrevento;
	list<const char *> *internos;
public:
	SCXMLEstadoCompuesto(const NodoXML *n,SCXMLEstado *p,SCXMLDataModel *d,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r);
	virtual ~SCXMLEstadoCompuesto();
	virtual bool soyYo(const char *estados) const;
	virtual bool corriendo(const char *estado) const;
	virtual bool esFinal() const;
	virtual void haTerminado(const char *hijo);
	virtual void termina();
	virtual void entrar();
	virtual void entrar(const char *hijo);
	virtual bool transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *eventdata);
	virtual list<TransicionActiva *> *evento(const char *n) const;
};

#endif