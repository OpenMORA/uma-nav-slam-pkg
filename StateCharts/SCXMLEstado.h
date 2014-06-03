#if !defined(AFX_SCXMLESTADO_H__3F2DACC4_2773_4618_A050_D87E0EB85F64__INCLUDED_)
#define AFX_SCXMLESTADO_H__3F2DACC4_2773_4618_A050_D87E0EB85F64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXML.h"
#include "SCXMLEjecutable.h"
#include "SCXMLTransition.h"
#include <list>
using namespace std;

typedef struct	{
	SCXMLTransition *trans;
	const char *saliente;
	const char *entrante;
}	TransicionActiva;

class SCXMLEstado	{
protected:
	char *nombre;
	SCXMLEjecutable **onentry;
	SCXMLEjecutable **onexit;
	SCXMLTransition **tConEventos;	//Transiciones con un atributo event
	SCXMLTransition **tSinEventos;	//Transiciones sin un atributo event pero con un atributo cond
	SCXMLTransition *tInicial;		//Transición sin atributo event ni cond. No puede haber más de una.
	SCXMLDataModel *datos;
	SCXMLEstado *padre;
	bool estaCorriendo;
	bool dmHijo;
public:

	//Método para crear cualquier estado, salvo la propia máquina de estados, que se crea llamando directamente a su constructor.
	static SCXMLEstado *crearEstado(const NodoXML *n,SCXMLEstado *p,SCXMLDataModel *dat,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r);

	//Devuelve la transición a ejecutar en caso de haberse recibido un evento.
	//Paralelo, compuesto y máquina tienen que heredarlo
	virtual list<TransicionActiva *> *evento(const char *n) const=0;
	
	/*Devuelve true si y sólo si se corresponde con un target válido dentro de este estado (nombre de un estado o subestado,
	  múltiples subestados dentro de un mismo estado paralelo. */
	virtual bool soyYo(const char *estados) const=0;

	/* Devuelve true si y sólo si el estado actual o un subestado del actual tiene como nombre a la cadena especificada, y además
	 * está ejecutándose. */
	virtual bool corriendo(const char *estado) const=0;

	/* Método que devuelve true si y sólo si el estado es final. */
	virtual bool esFinal() const=0;

	/* Este método es llamado por los hijos del estado actual, para indicarle que han acabado de forma ajena a una transición. */
	virtual void haTerminado(const char *hijo)=0;

	//Método llamado desde el hijo para indicar que debe terminar (hay una sutil diferencia con el método anterior).
	//También es llamado desde un estado paralelo a sus hijos.
	virtual void termina();

	virtual void entrar();

	//Entrar a un hijo de este nodo.
	virtual void entrar(const char *hijo)=0;

	//Transición iniciada por el hijo
	virtual bool transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *eventdata)=0;

	const char *getNombre() const;
};
#endif