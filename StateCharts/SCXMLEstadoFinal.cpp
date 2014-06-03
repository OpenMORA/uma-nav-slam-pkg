#include "SCXMLEstadoFinal.h"

SCXMLEstadoFinal::SCXMLEstadoFinal(const NodoXML *n,SCXMLEstado *p)	{
	nombre=_strdup(SCXML::atributo(n,"id"));
	onentry=NULL;
	onexit=NULL;
	tConEventos=new SCXMLTransition *;
	*tConEventos=NULL;
	tSinEventos=new SCXMLTransition *;
	*tSinEventos=NULL;
	tInicial=NULL;
	datos=NULL;
	padre=p;
	estaCorriendo=false;
}

SCXMLEstadoFinal::~SCXMLEstadoFinal()	{
	//No debería tener onentry, onexit, tConEventos, tSinEventos, tInicial ni datos. No hay nada que hacer,
	if (nombre) free(nombre);
	delete tConEventos;
	delete tSinEventos;
}

bool SCXMLEstadoFinal::soyYo(const char *estados) const	{
	return !strcmp(estados,nombre);
}

bool SCXMLEstadoFinal::corriendo(const char *estado) const	{
	return estaCorriendo&&!strcmp(estado,nombre);
}

void SCXMLEstadoFinal::entrar()	{
	SCXMLEstado::entrar();
	padre->haTerminado(nombre);
}

bool SCXMLEstadoFinal::esFinal() const	{
	return true;
}

void SCXMLEstadoFinal::entrar(const char *hijo)	{
	if (!strcmp(hijo,nombre)) entrar();
}

list<TransicionActiva *> *SCXMLEstadoFinal::evento(const char *n) const	{
	return new list<TransicionActiva *>;
}

void SCXMLEstadoFinal::haTerminado(const char *hijo)	{}

bool SCXMLEstadoFinal::transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *d)	{
	return true;
}