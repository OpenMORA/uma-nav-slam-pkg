#if !defined(AFX_SCXMLESTADOFINAL_H__780CA1F9_F302_41B4_A2EC_422E29FA1E54__INCLUDED_)
#define AFX_SCXMLESTADOFINAL_H__780CA1F9_F302_41B4_A2EC_422E29FA1E54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEstado.h"

class SCXMLEstadoFinal:public SCXMLEstado	{
public:
	SCXMLEstadoFinal(const NodoXML *n,SCXMLEstado *p);
	virtual ~SCXMLEstadoFinal();
	virtual void entrar();
	virtual bool corriendo(const char *nombre) const;
	virtual bool soyYo(const char *estados) const;
	virtual bool esFinal() const;
	virtual void haTerminado(const char *hijo);
	virtual void entrar(const char *hijo);
	virtual bool transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *d);
	virtual list<TransicionActiva *> *evento(const char *n) const;
};
#endif