#include "SCXMLTransition.h"
#include "SCXMLEvent.h"

SCXMLTransition::SCXMLTransition(const NodoXML *n,SCXMLDataModel *d,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r)	{
	modelo=d;
	const char *tmp=SCXML::atributo(n,"event");
	event=tmp?_strdup(tmp):NULL;
	tmp=SCXML::atributo(n,"cond");
	cond=tmp?SCXMLCondicion::crearCondicion(tmp):NULL;
	tmp=SCXML::atributo(n,"target");
	target=tmp?_strdup(tmp):NULL;
	sentencias=new SCXMLEjecutable*[n->cuantos+2];
	for (int i=0;i<n->cuantos;i++) sentencias[i]=SCXMLEjecutable::crearEjecutable(n->hijos+i,lI,lE,r);
	sentencias[n->cuantos]=new SCXMLEvent(NULL,lI);
	sentencias[n->cuantos+1]=NULL;
}

SCXMLTransition::~SCXMLTransition()	{
	if (event) free(event);
	delete cond;
	if (target) free(target);
	for (SCXMLEjecutable **p=sentencias;*p;p++) delete *p;
	delete[] sentencias;
}

void SCXMLTransition::ejecutar(SCXMLData *eventData)	{
	if (eventData)	{
		SCXMLDataModel *d=new SCXMLDataModel(eventData,"_eventData",modelo);
		for (SCXMLEjecutable **p=sentencias;*p;p++) (*p)->ejecutar(d);
		delete d;
	}	else for (SCXMLEjecutable **p=sentencias;*p;p++) (*p)->ejecutar(modelo);
}

bool SCXMLTransition::activar() const	{
	if (!cond) return true;
	else return cond->evaluar(modelo);
}

bool SCXMLTransition::activar(const char *evento) const	{	//true si la transición se activa (hay que fijarse también en el datamodel)
	if (!evento) return activar();
	if (!strcmp(evento,event)) return activar();
	return false;
}

const char *SCXMLTransition::getTarget() const	{
	return target;
}