#include "SCXMLMaquinaDeEstados.h"

inline SCXMLEstado *hijoC(SCXMLEstado **h,const char *n)	{
	if (!n) return NULL;
	for (SCXMLEstado **p=h;*p;p++) if (!strcmp((*p)->getNombre(),n)) return *p;
	return NULL;
}

void SCXMLMaquinaDeEstados::manejaEventos()	{
	for (;;)	{
		mrpt::system::sleep(100); //cga 08/02/2010
		
		if (stop)	{
			parado=true;
			//ExitThread(0);
			return;
		}
	//	printf("ManejaEventos\n");
		if (internos->size()>0)	{
		//	printf("inside manejaEventos\n");
			const char *nE=internos->front();
			internos->pop_front();
			list<TransicionActiva *> *listaTrans=evento(nE);
			if (!listaTrans) continue;
			else if (listaTrans->empty())	{
				delete listaTrans;
				continue;
			}
			for (list<TransicionActiva *>::iterator i=listaTrans->begin();i!=listaTrans->end();i++)	{
				transicion((*i)->saliente,(*i)->entrante,(*i)->trans,NULL);
				delete *i;
			}
			listaTrans->clear();
			delete listaTrans;
		}
		mutex.enter();
		bool res=entrantes->size()>0;
		mutex.leave();
		if (res)	{
			mutex.enter();
			EventoExterno *e=entrantes->front();
			entrantes->pop_front();
			mutex.leave();
			list<TransicionActiva *> *listaTrans=evento(e->nombre);
			if (!listaTrans) continue;
			else if (listaTrans->empty())	{
				delete listaTrans;
				e->procesados++;
				continue;
			}
			for (list<TransicionActiva *>::iterator i=listaTrans->begin();i!=listaTrans->end();i++)	{
				transicion((*i)->saliente,(*i)->entrante,(*i)->trans,e->_eventData);
				delete *i;
			}
			listaTrans->clear();
			delete listaTrans;
			e->procesados++;
		}
	}
}

void threadFunc(SCXMLMaquinaDeEstados *p)
{
	p->manejaEventos();
}

list<TransicionActiva *> *SCXMLMaquinaDeEstados::evento(const char *n) const	{
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
	}	else for (SCXMLTransition **p=tConEventos;*p;p++)	{
		if ((*p)->activar(n))	{
			TransicionActiva *t=new TransicionActiva;
			t->trans=*p;
			t->saliente=nombre;
			t->entrante=(*p)->getTarget();
			list<TransicionActiva *> *l=new list<TransicionActiva *>;
			l->push_back(t);
			return l;
		}
	}
	if (actual) return hijoC(hijos,actual)->evento(n);
	return new list<TransicionActiva *>;
}

SCXMLMaquinaDeEstados::SCXMLMaquinaDeEstados(const NodoXML *n,list<EventoExterno *> *lE,Rutinas *r,mrpt::synch::CCriticalSection &m)
	: mutex(m)
{
	rut=r;
	salientes=lE;
	entrantes=new list<EventoExterno *>;
	internos=new list<const char *>;
	datos=NULL;
	estaCorriendo=false;
	nombre=_strdup("top");
	padre=NULL;
	eInicial=_strdup(SCXML::atributo(n,"initialstate"));
	int nEstados=0;
	int nTC=0;
	int nTS=0;
	for (int i=0;i<n->cuantos;i++) if (!strcmp(n->hijos[i].nombre,"state")||!strcmp(n->hijos[i].nombre,"parallel")||!strcmp(n->hijos[i].nombre,"final")) nEstados++;
	else if (!strcmp(n->hijos[i].nombre,"transition"))	
	{
		printf("Checking Transitions\n");
		if (SCXML::atributo(n->hijos+i,"event")) nTC++;
		else if (SCXML::atributo(n->hijos+i,"cond")) nTS++;
		else throw runtime_error("Las máquinas de estados no pueden contener transiciones sin eventos ni condiciones.");
	}
	tConEventos=new SCXMLTransition*[nTC+1];
	tSinEventos=new SCXMLTransition*[nTS+1];
	int iTC=0;
	int iTS=0;
	//printf("There are %d states.\n",nEstados);
	hijos=new SCXMLEstado*[nEstados+1];
	int indice=0;
	//printf("-->%d\n",n->cuantos);
	for (int i=0;i<n->cuantos;i++) 
		if (!strcmp(n->hijos[i].nombre,"state")||!strcmp(n->hijos[i].nombre,"parallel")||!strcmp(n->hijos[i].nombre,"final")) 
		{
	//		printf("indice %d, %s %d\n",indice,(n->hijos+i)->nombre,(n->hijos+i)->cuantos);
			hijos[indice++]=SCXMLEstado::crearEstado(n->hijos+i,this,datos,internos,salientes,rut);

		}	
	else if (!strcmp(n->hijos[i].nombre,"datamodel"))	{
		if (datos) throw runtime_error("Hay más de un datamodel raíz.\n");
		datos=new SCXMLDataModel(n->hijos+i,NULL,rut);
	}	else if (!strcmp(n->hijos[i].nombre,"transition"))	{
		printf("Creating a transition\n");
		SCXMLTransition *t=new SCXMLTransition(n->hijos+i,datos,internos,lE,rut);
		if (SCXML::atributo(n->hijos+i,"event")) tConEventos[iTC++]=t;
		else tSinEventos[iTS++]=t;
	}
	tConEventos[nTC]=NULL;
	tSinEventos[nTS]=NULL;   //it was nTC!! by cga
	hijos[nEstados]=NULL;
	//printf("%d,%d\n",iTS,iTC);
	stop=false;
}

SCXMLMaquinaDeEstados::~SCXMLMaquinaDeEstados()	{
	free(nombre);
	free(eInicial);
	entrantes->clear();	//¡NO BORRES! Eso ya lo hace el ManejadorSCXML que tienes encima.
	delete entrantes;
	internos->clear();
	delete internos;
}

bool SCXMLMaquinaDeEstados::corriendo(const char *c) const	{
	if (!estaCorriendo) return false;
	for (SCXMLEstado **p=hijos;*p;p++) if ((*p)->soyYo(actual)) return (*p)->corriendo(c);
	return false;
}

void SCXMLMaquinaDeEstados::entrar()	{
	bool entrado=false;
	for (SCXMLEstado **p=hijos;*p;p++) if ((*p)->soyYo(eInicial))	{
		(*p)->entrar(eInicial);
		actual=(*p)->getNombre();
		//printf("Entrando in %s\n",actual);
		entrado=true;
		break;
	}
	if (!entrado) throw runtime_error("Máquina de estados mal construida, no se encuentra el estado inicial.");
	estaCorriendo=true;
	mrpt::system::createThread(threadFunc,this);
}

void SCXMLMaquinaDeEstados::entrar(const char *c)	{
	actual=NULL;
	for (SCXMLEstado **p=hijos;*p;p++) if ((*p)->soyYo(c))	{
		(*p)->entrar(c);
		actual=(*p)->getNombre();
		break;
	}
}

bool SCXMLMaquinaDeEstados::esFinal() const	{
	return false;
}

void SCXMLMaquinaDeEstados::haTerminado(const char *c)	{
	EventoExterno *e=new EventoExterno;
	e->_eventData=NULL;
	e->nombre="top.DONE";
	e->target=NULL;
	e->procesados=0;
	e->tstamp= mrpt::system::now();
	mutex.enter();
	salientes->push_back(e);
	mutex.leave();
	//Eliminación de la hebra (que, no olvidemos, ES LA HEBRA QUE EJECUTA ESTE MÉTODO).
	parado=true;
	//ExitThread(0);
	mrpt::system::exitThread();
}

bool SCXMLMaquinaDeEstados::soyYo(const char *estados) const	{
	return false;	//Este método nunca debería ser llamado, ya que sólo lo mandan los padres, y este estado no tiene.
}

void SCXMLMaquinaDeEstados::termina()	{
	haTerminado(NULL);
}

bool SCXMLMaquinaDeEstados::transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *eventdata)	{
	if (!hacia)	{
		t->ejecutar(eventdata);
		return false;
	}
	SCXMLEstado *a=hijoC(hijos,actual);
	if (a->transicion(desde,hacia,t,eventdata))	{
		SCXMLEstado *h=NULL;
		for (SCXMLEstado **p=hijos;*p;p++) if ((*p)->soyYo(hacia))	{
			h=*p;
			break;
		}
		if (!h) throw runtime_error("Error gestionando los estados.");
		t->ejecutar(eventdata);
		h->entrar(hacia);
		actual=h->getNombre();
	}
	return false;
}

list<EventoExterno *> *SCXMLMaquinaDeEstados::colaEventos()	{
	return entrantes;
}

void SCXMLMaquinaDeEstados::parar()	{
	stop=true;
	while (!parado)
	{
		mrpt::system::sleep(100);
	}
}

void SCXMLMaquinaDeEstados::reanudar()	{
	stop=false;
}