#if !defined(AFX_SCXMLMAQUINADEESTADOS_H__3C025272_2833_4DFA_82A3_A87D7736B3B1__INCLUDED_)
#define AFX_SCXMLMAQUINADEESTADOS_H__3C025272_2833_4DFA_82A3_A87D7736B3B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <windows.h>
#include <mrpt/synch.h>

#include "SCXML.h"
#include "SCXMLEstado.h"
#include <list>
using namespace std;

class SCXMLMaquinaDeEstados:public SCXMLEstado	{
private:
	list<EventoExterno *> *salientes;	//Recibe eventos de aquí (los mete una clase desde fuera).
	list<EventoExterno *> *entrantes;	//Para que los estados hijos metan eventos (una clase desde fuera los leerá).
	list<const char *> *internos;
	char *eInicial;
	const char *actual;
	SCXMLEstado **hijos;
	Rutinas *rut;
	//HANDLE mutex;
	mrpt::synch::CCriticalSection &mutex;
	bool stop;
	bool parado;
public:
	SCXMLMaquinaDeEstados(const NodoXML *n,list<EventoExterno *> *lE,Rutinas *r,mrpt::synch::CCriticalSection & m);
	virtual ~SCXMLMaquinaDeEstados();
	list<EventoExterno *> *colaEventos();
	virtual bool corriendo(const char *c) const;
	virtual void entrar();
	virtual void entrar(const char *e);
	virtual bool esFinal() const;
	virtual void haTerminado(const char *hijo);
	virtual bool soyYo(const char *estados) const;
	virtual void termina();
	virtual bool transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *eventdata);
	virtual list<TransicionActiva *> *evento(const char *n) const;
	void manejaEventos();
	void parar();
	void reanudar();
};
#endif
