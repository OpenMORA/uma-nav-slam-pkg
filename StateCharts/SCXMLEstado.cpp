#include "SCXMLEstado.h"
#include "SCXMLEstadoFinal.h"
#include "SCXMLEstadoSimple.h"
#include "SCXMLEstadoCompuesto.h"
#include "SCXMLEstadoParalelo.h"

void SCXMLEstado::entrar()	{
	estaCorriendo=true;
	//printf("Entrando en entrar estado\n");
	if (onentry) for (SCXMLEjecutable **p=onentry;*p;p++) (*p)->ejecutar(datos);
}

void SCXMLEstado::termina()	{
	if (!estaCorriendo) return;
	if (onexit) for (SCXMLEjecutable **p=onexit;*p;p++) (*p)->ejecutar(datos);
	estaCorriendo=false;
}

SCXMLEstado *SCXMLEstado::crearEstado(const NodoXML *n,SCXMLEstado *p,SCXMLDataModel *dat,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r)	{
	if (!strcmp(n->nombre,"final")) return new SCXMLEstadoFinal(n,p);
	if (!strcmp(n->nombre,"parallel")) return new SCXMLEstadoParalelo(n,p,dat,lI,lE,r);
	if (strcmp(n->nombre,"state")) throw runtime_error("El nodo no se corresponde con un estado");
	for (int i=0;i<n->cuantos;i++)	{
		if (!strcmp(n->hijos[i].nombre,"state")||!strcmp(n->hijos[i].nombre,"parallel")||!strcmp(n->hijos[i].nombre,"final")) return new SCXMLEstadoCompuesto(n,p,dat,lI,lE,r);
		if (!strcmp(n->hijos[i].nombre,"invoke")) return NULL;	//¿TODO: crear estado Invoke?
	}
	//printf("Creating state %s with %d\n",n->nombre,n->cuantos);
	return new SCXMLEstadoSimple(n,p,dat,lI,lE,r);
}

const char *SCXMLEstado::getNombre() const	{
	return nombre;
}
