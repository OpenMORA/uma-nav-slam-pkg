#include "SCXMLEstadoSimple.h"

void anadirTransicion(SCXMLTransition *que,SCXMLTransition **&donde)	{
	int n=0;
	for (SCXMLTransition **p=donde;*p;p++) n++;
	SCXMLTransition **res=new SCXMLTransition*[n+2];
	for (int i=0;i<n;i++) res[i]=donde[i];
	res[n]=que;
	res[n+1]=NULL;
	delete[] donde;
	donde=res;
}

SCXMLEstadoSimple::SCXMLEstadoSimple(const NodoXML *n,SCXMLEstado *p,SCXMLDataModel *d,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r)	{
	nombre=_strdup(SCXML::atributo(n,"id"));
	tConEventos=new SCXMLTransition*;
	*tConEventos=NULL;
	tSinEventos=new SCXMLTransition*;
	*tSinEventos=NULL;
	tInicial=NULL;
	onentry=NULL;
	onexit=NULL;
	datos=NULL;
	dmHijo=false;
	estaCorriendo=false;
	//printf("Before here there are %d\n",n->cuantos);
	for (int i=0;i<n->cuantos;i++) if (!strcmp(n->hijos[i].nombre,"onentry"))	{
		if (onentry) throw runtime_error("Se ha encontrado un estado con más de un grupo onentry.");
		onentry=new SCXMLEjecutable*[n->hijos[i].cuantos+1];
		for (int j=0;j<n->hijos[i].cuantos;j++) onentry[j]=SCXMLEjecutable::crearEjecutable(n->hijos[i].hijos+j,lI,lE,r);
		onentry[n->hijos[i].cuantos]=NULL;
	}	else if (!strcmp(n->hijos[i].nombre,"onexit"))	{
		if (onexit) throw runtime_error("Se ha encontrado un estado con más de un grupo onexit.");
		onexit=new SCXMLEjecutable*[n->hijos[i].cuantos+1];
		for (int j=0;j<n->hijos[i].cuantos;j++) onexit[j]=SCXMLEjecutable::crearEjecutable(n->hijos[i].hijos+j,lI,lE,r);
		onexit[n->hijos[i].cuantos]=NULL;
	}	else if (!strcmp(n->hijos[i].nombre,"transition"))	{
		SCXMLTransition *t=new SCXMLTransition(n->hijos+i,datos?datos:d,lI,lE,r);
		if (SCXML::atributo(n->hijos+i,"event")) anadirTransicion(t,tConEventos);
		else if (SCXML::atributo(n->hijos+i,"cond")) anadirTransicion(t,tSinEventos);
		else if (tInicial) throw new runtime_error("Se ha encontrado un estado con más de una transición inicial");
		else tInicial=t;
	}	else if (!strcmp(n->hijos[i].nombre,"datamodel"))	{
		dmHijo=true;
		if (datos) throw runtime_error("Se ha encontrado un estado con más de un datamodel.");
		datos=new SCXMLDataModel(n->hijos+i,d);
	}
	if (!datos) datos=d;
}

SCXMLEstadoSimple::~SCXMLEstadoSimple()	{
	free(nombre);
	SCXMLTransition ** p;
	for (p=tConEventos;*p;p++) delete *p;
	delete[] tConEventos;
	for (p=tSinEventos;*p;p++) delete *p;
	delete[] tSinEventos;
	if (tInicial) delete tInicial;
	if (onentry)	{
		for (SCXMLEjecutable **p2=onentry;*p2;p2++) delete *p2;
		delete[] onentry;
	}
	if (onexit)	{
		for (SCXMLEjecutable **p2=onexit;*p2;p2++) delete *p2;
		delete[] onexit;
	}
	if (dmHijo) delete datos;	//Si no, el datamodel viene del padre, y se liberará más arriba.
}

bool SCXMLEstadoSimple::soyYo(const char *estados) const	{
	return !strcmp(estados,nombre);
}

bool SCXMLEstadoSimple::corriendo(const char *estado) const	{
	return !strcmp(estado,nombre)&&estaCorriendo;
}

bool SCXMLEstadoSimple::esFinal() const	{
	return false;
}

void SCXMLEstadoSimple::entrar(const char *hijo)	{
	if (!strcmp(hijo,nombre)) SCXMLEstado::entrar();
}

list<TransicionActiva *> *SCXMLEstadoSimple::evento(const char *n) const	{
	list<TransicionActiva *> *l=new list<TransicionActiva *>;
	if (!n)	{
		if (tInicial)	{
			TransicionActiva *t=new TransicionActiva;
			t->trans=tInicial;
			t->saliente=nombre;
			t->entrante=tInicial->getTarget();
			l->push_back(t);
		}
		for (SCXMLTransition **p=tSinEventos;*p;p++)	{
			if ((*p)->activar(NULL))	{
				TransicionActiva *t=new TransicionActiva;
				t->trans=*p;
				t->saliente=nombre;
				t->entrante=(*p)->getTarget();
				l->push_back(t);
				return l;
			}
		}
	}	else for (SCXMLTransition **p=tConEventos;*p;p++) if ((*p)->activar(n))	{
		TransicionActiva *t=new TransicionActiva;
		t->trans=*p;
		t->saliente=nombre;
		t->entrante=(*p)->getTarget();
		l->push_back(t);
		return l;
	}
	return l;
}

void SCXMLEstadoSimple::haTerminado(const char *h)	{}

bool SCXMLEstadoSimple::transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *d)	{
	if (!strcmp(nombre,desde))	{
		termina();
		if (!strcmp(nombre,hacia))	{
			t->ejecutar(d);
			SCXMLEstado::entrar();
			return false;
		}	else return true;
	}	else return false;
}