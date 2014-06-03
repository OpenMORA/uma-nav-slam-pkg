#if !defined(AFX_SCXMLESTADOPARALELO_H__1BB9936B_9356_49E7_9134_8FB4793E6DCA__INCLUDED_)
#define AFX_SCXMLESTADOPARALELO_H__1BB9936B_9356_49E7_9134_8FB4793E6DCA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEstado.h"

class SCXMLEstadoParalelo:public SCXMLEstado	{
private:
	SCXMLEstado **hijos;
	const char **actuales;	//Valdrá NULL siempre que el estado no esté activo
	list<const char *> *internos;
	char *nombrevento;
public:
	SCXMLEstadoParalelo(const NodoXML *n,SCXMLEstado *p,SCXMLDataModel *d,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r);
	virtual ~SCXMLEstadoParalelo();
	virtual bool soyYo(const char *estado) const;
	virtual bool corriendo(const char *estado) const;
	virtual bool esFinal() const;
	virtual void haTerminado(const char *estado);
	virtual void termina();
	virtual void termina(const char *terminante);
	virtual void entrar();
	virtual void entrar(const char *estados);
	virtual bool transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *eventdata);
	virtual list<TransicionActiva *> *evento(const char *n) const;
};
#endif