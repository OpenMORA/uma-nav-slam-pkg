	#include "SCXMLEstadoCompuesto.h"

inline SCXMLEstado *hijoC(SCXMLEstado **h,const char *n)	{
	if (!n) return NULL;
	for (SCXMLEstado **p=h;*p;p++) if (!strcmp((*p)->getNombre(),n)) return *p;
	return NULL;
}

SCXMLEstadoCompuesto::SCXMLEstadoCompuesto(const NodoXML *n,SCXMLEstado *p,SCXMLDataModel *d,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r)	{
	internos=lI;
	datos=NULL;
	dmHijo=false;
	estaCorriendo=false;
	nombre=_strdup(SCXML::atributo(n,"id"));
	onentry=NULL;
	onexit=NULL;
	padre=p;
	tInicial=NULL;
	inicial=NULL;
	int nHijos=1;
	int nTS=1;
	int nTC=1;
	for (int i=0;i<n->cuantos;i++)	{
		if (!strcmp(n->hijos[i].nombre,"state")||!strcmp(n->hijos[i].nombre,"parallel")||!strcmp(n->hijos[i].nombre,"final")) nHijos++;
		if (!strcmp(n->hijos[i].nombre,"transition"))	{
			if (SCXML::atributo(n->hijos+i,"event")) nTC++;
			else if (SCXML::atributo(n->hijos+i,"cond")) nTS++;
			else throw runtime_error("Los estados compuestos no pueden tener transiciones sin eventos ni condiciones.");
		}
	}
	hijos=new SCXMLEstado*[nHijos];
	tConEventos=new SCXMLTransition*[nTC];
	tSinEventos=new SCXMLTransition*[nTS];
	int indiceH=0;
	int indiceC=0;
	int indiceS=0;
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
	}	else if (!strcmp(n->hijos[i].nombre,"initial"))	{
		if (inicial) throw runtime_error("Se ha encontrado un estado compuesto con más de un estado inicial.");
		for (int j=0;j<n->hijos[i].cuantos;j++) if (!strcmp(n->hijos[i].hijos[j].nombre,"transition"))	{
			tInicial=new SCXMLTransition(n->hijos[i].hijos+j,datos?datos:d,lI,lE,r);
			break;
		}
		if (!tInicial)	{
			SCXML::mostrar(n);
			throw runtime_error("No se ha encontrado la transición inicial.");
		}
		inicial=_strdup(tInicial->getTarget());
	}	else if (!strcmp(n->hijos[i].nombre,"state")||!strcmp(n->hijos[i].nombre,"parallel")||!strcmp(n->hijos[i].nombre,"final")) hijos[indiceH++]=SCXMLEstado::crearEstado(n->hijos+i,this,datos?datos:d,lI,lE,r);
	else if (!strcmp(n->hijos[i].nombre,"datamodel"))	{
		if (datos) throw runtime_error("Se ha encontrado un estado con más de un datamodel.");
		datos=new SCXMLDataModel(n->hijos+i,d);
		dmHijo=true;
	}	else if (!strcmp(n->hijos[i].nombre,"transition"))	{
		SCXMLTransition *t=new SCXMLTransition(n->hijos+i,datos?datos:d,lI,lE,r);
		if (SCXML::atributo(n->hijos+i,"event")) tConEventos[indiceC++]=t;
		else tSinEventos[indiceS++]=t;
	}
	hijos[nHijos-1]=NULL;
	tConEventos[nTC-1]=NULL;
	tSinEventos[nTS-1]=NULL;
	if (!inicial) throw runtime_error("No se ha encontrado el estado inicial de un estado compuesto.");
	const char *tI=tInicial->getTarget();
	if (!hijoC(hijos,tI)) throw runtime_error("La transición inicial es incorrecta.");
	if (!datos) datos=d;
	actual=NULL;
	nombrevento=new char[strlen(nombre)+6];
	strcpy(nombrevento,nombre);
	strcat(nombrevento,".DONE");
}

SCXMLEstadoCompuesto::~SCXMLEstadoCompuesto()	{
	delete inicial;
	for (SCXMLEstado **p=hijos;*p;p++) delete *p;
	delete[] hijos;
	delete nombre;
	if (dmHijo) delete datos;
	if (onentry)	{
		for (SCXMLEjecutable **p2=onentry;*p2;p2++) delete *p2;
		delete[] onentry;
	}
	if (onexit)	{
		for (SCXMLEjecutable **p2=onentry;*p2;p2++) delete *p2;
		delete[] onexit;
	}
	for (SCXMLTransition **p2=tConEventos;*p2;p2++) delete *p2;
	delete[] tConEventos;
	for (SCXMLTransition **p2=tSinEventos;*p2;p2++) delete *p2;
	delete[] tSinEventos;
	delete tInicial;
	delete nombrevento;
}

bool SCXMLEstadoCompuesto::soyYo(const char *estados) const	{
	if (!strcmp(estados,nombre)) return true;
	if (!strcmp(estados,inicial)) return true;
	for (SCXMLEstado **p=hijos;*p;p++) if ((*p)->soyYo(estados)) return true;
	return false;
}

bool SCXMLEstadoCompuesto::corriendo(const char *estado) const	{
	if (!estaCorriendo) return false;
	if (!strcmp(estado,nombre)) return true;
	SCXMLEstado *h=hijoC(hijos,actual);
	if (h) return h->corriendo(estado);
	else return false;
}

bool SCXMLEstadoCompuesto::esFinal() const	{
	return false;
}

void SCXMLEstadoCompuesto::haTerminado(const char *hijo)	{
	//Sólo hay un estado hijo, de manera que si éste termina, el padre también.
	actual=NULL;
	termina();
	internos->push_back(nombrevento);
	padre->haTerminado(nombre);
}

void SCXMLEstadoCompuesto::termina()	{
	SCXMLEstado *h=hijoC(hijos,actual);
	if (h) h->termina();
	if (onexit) for (SCXMLEjecutable **p=onexit;*p;p++) (*p)->ejecutar(datos);
	estaCorriendo=false;
}

void SCXMLEstadoCompuesto::entrar()	{
	if (onentry) for (SCXMLEjecutable **p=onentry;*p;p++) (*p)->ejecutar(datos);
	const char *t=tInicial->getTarget();
	SCXMLEstado *h=hijoC(hijos,t);
	tInicial->ejecutar(NULL);
	h->entrar(t);
	actual=t;
	estaCorriendo=true;
}

void SCXMLEstadoCompuesto::entrar(const char *hijo)	{
	if (!strcmp(hijo,nombre)) entrar();
	else for (SCXMLEstado **p=hijos;*p;p++) if ((*p)->soyYo(hijo))	{
		(*p)->entrar(hijo);
		break;
	}
	estaCorriendo=true;
}

bool SCXMLEstadoCompuesto::transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *eventdata)	{
	if (!strcmp(desde,nombre))	{
		if (!strcmp(hacia,actual))	{
			termina();
			t->ejecutar(eventdata);
			entrar();
			return false;
		}	else if (soyYo(hacia)) throw runtime_error("Máquina de estados mal construida.\n");
		else	{
			termina();
			return true;
		}
	}
	SCXMLEstado *act=hijoC(hijos,actual);
	if (!act->soyYo(desde)) return false;	//Esto nunca debe ocurrir, en realidad.
	if (act->transicion(desde,hacia,t,eventdata))	{
		//Queda trabajo por hacer
		if (soyYo(hacia))	{
			t->ejecutar(eventdata);
			SCXMLEstado *nuevo=NULL;
			for (SCXMLEstado **p=hijos;*p;p++) if ((*p)->soyYo(hacia))	{
				nuevo=*p;
				break;
			}
			if (!nuevo) throw runtime_error("Error desconocido gestionando los estados.");
			nuevo->entrar(hacia);
			actual=nuevo->getNombre();
			return false;
		}	else	{
			actual=NULL;
			termina();
			return true;
		}
	}	else return false;
}

list<TransicionActiva *> *SCXMLEstadoCompuesto::evento(const char *n) const	{
	if (!n)	{
		for (SCXMLTransition **p=tSinEventos;*p;p++) if ((*p)->activar(NULL))	{
			TransicionActiva *t=new TransicionActiva;
			t->trans=*p;
			t->saliente=nombre;
			t->entrante=(*p)->getTarget();
			list<TransicionActiva *> *l=new list<TransicionActiva *>;
			l->push_back(t);
			return l;
		}
	}	else for (SCXMLTransition **p=tConEventos;*p;p++) if ((*p)->activar(n))	{
		TransicionActiva *t=new TransicionActiva;
		t->trans=*p;
		t->saliente=nombre;
		t->entrante=(*p)->getTarget();
		list<TransicionActiva *> *l=new list<TransicionActiva *>;
		l->push_back(t);
		return l;
	}
	if (actual) return hijoC(hijos,actual)->evento(n);
	return new list<TransicionActiva *>;
}
