#include "SCXMLEstadoParalelo.h"

SCXMLEstadoParalelo::SCXMLEstadoParalelo(const NodoXML *n,SCXMLEstado *p,SCXMLDataModel *d,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r)	{
	internos=lI;
	onentry=NULL;
	onexit=NULL;
	nombre=_strdup(SCXML::atributo(n,"id"));
	datos=NULL;
	int nEstados=0;
	int nTransC=0;
	int nTransS=0;
	for (int i=0;i<n->cuantos;i++) if (!strcmp(n->hijos[i].nombre,"state")||!strcmp(n->hijos[i].nombre,"parallel")) nEstados++;
	else if (!strcmp(n->hijos[i].nombre,"transition"))	{
		if (SCXML::atributo(n->hijos+i,"event")) nTransC++;
		else if (SCXML::atributo(n->hijos+i,"cond")) nTransS++;
		else throw runtime_error("Los estados compuestos no pueden tener transiciones sin eventos ni condiciones.");
	}
	hijos=new SCXMLEstado*[nEstados+1];
	tInicial=NULL;
	tConEventos=new SCXMLTransition*[nTransC+1];
	tSinEventos=new SCXMLTransition*[nTransS+1];
	int iHijos=0;
	int iTC=0;
	int iTS=0;
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
	}	else if (!strcmp(n->hijos[i].nombre,"state")||!strcmp(n->hijos[i].nombre,"parallel"))	hijos[iHijos++]=SCXMLEstado::crearEstado(n->hijos+i,this,datos?datos:d,lI,lE,r);
	else if (!strcmp(n->hijos[i].nombre,"datamodel"))	{
		if (datos) throw runtime_error("Se ha encontrado un estado con más de un datamodel.");
		datos=new SCXMLDataModel(n->hijos+i,d);
		dmHijo=true;
	}	else if (!strcmp(n->hijos[i].nombre,"transition"))	{
		SCXMLTransition *t=new SCXMLTransition(n->hijos+i,datos?datos:d,lI,lE,r);
		if (SCXML::atributo(n->hijos+i,"event")) tConEventos[iTC++]=t;
		else tSinEventos[iTS++]=t;
	}
	if (!datos) datos=d;
	hijos[nEstados]=NULL;
	tConEventos[nTransC]=NULL;
	tSinEventos[nTransS]=NULL;
	actuales=NULL;
	nombrevento=new char[strlen(nombre)+6];
	strcpy(nombrevento,nombre);
	strcat(nombrevento,".DONE");
}

SCXMLEstadoParalelo::~SCXMLEstadoParalelo()	{
	for (SCXMLEstado **p=hijos;*p;p++) delete *p;
	delete[] hijos;
	delete nombre;
	if (dmHijo) delete datos;
	if (onentry)	{
		for (SCXMLEjecutable **p2=onentry;*p2;p2++) delete *p2;
		delete[] onentry;
	}
	if (onexit)	{
		for (SCXMLEjecutable **p2=onexit;*p2;p2++) delete *p2;
		delete[] onexit;
	}
	for (SCXMLTransition **p2=tConEventos;*p2;p2++) delete *p2;
	delete[] tConEventos;
	for (SCXMLTransition **p2=tSinEventos;*p2;p2++) delete *p2;
	delete[] tSinEventos;
	delete nombrevento;
}

list<char *> *mistrtok(const char *c,const char *bus)	{
	list<char *> *res=new list<char *>;
	c+=strspn(c,bus);
	const char *p=c;
	//int n=0;
	for (;;)	{
		size_t sp=strcspn(p,bus);
		if (!sp) break;
		char *m=new char[sp+1];
		strncpy(m,p,sp);
		m[sp]='\0';
		res->push_back(m);
		p+=sp;
		p+=strspn(p,bus);
	}
	return res;
}

bool SCXMLEstadoParalelo::soyYo(const char *estado) const	{
	if (!strcmp(estado,nombre)) return true;
	list<char *> *m=mistrtok(estado,", ");
	bool siX=true;
	for (list<char *>::iterator i=m->begin();i!=m->end();i++)	{
		bool si=false;
		for (SCXMLEstado **s=hijos;*s;s++) if ((*s)->soyYo(*i))	{
			si=true;
			break;
		}
		delete[] *i;
		siX=siX&&si;
	}
	delete m;
	return siX;
}

SCXMLEstado *hijoP(SCXMLEstado **h,const char *n)	{
	for (SCXMLEstado **p=h;*p;p++) if ((*p)->soyYo(n)) return *p;
	return NULL;
}

bool SCXMLEstadoParalelo::corriendo(const char *estado) const	{
	if (!estaCorriendo) return false;
	if (!strcmp(estado,nombre)) return true;
	SCXMLEstado *h=hijoP(hijos,estado);
	if (!h) return false;
	return h->corriendo(estado);
}

bool SCXMLEstadoParalelo::esFinal() const	{
	return false;
}

void SCXMLEstadoParalelo::haTerminado(const char *estado)	{
	int pos=-1;
	int cuantos=0;
	for (const char **p=actuales;*p;p++)	{
		if (!strcmp(*p,estado)) pos=cuantos;
		cuantos++;
	}
	if (pos==-1) return;
	const char **p2=new const char*[cuantos];
	for (int i=0;i<pos;i++) p2[i]=actuales[i];
	for (int i=pos;i<cuantos;i++) p2[i]=actuales[i+1];
	p2[cuantos]=NULL;
	delete[] actuales;
	actuales=p2;
	if (cuantos==1)	{
		termina();
		internos->push_back(nombrevento);
	}
}

void SCXMLEstadoParalelo::termina(const char *terminante)	{
	estaCorriendo=false;
	for (const char **p=actuales;*p;p++) if (strcmp(*p,terminante))	{
		SCXMLEstado *h=hijoP(hijos,*p);
		h->termina();
	}
	if (onexit) for (SCXMLEjecutable **p2=onexit;*p2;p2++) (*p2)->ejecutar(datos);
	delete[] actuales;
	actuales=NULL;
}

void SCXMLEstadoParalelo::termina()	{
	estaCorriendo=false;
	for (const char **p=actuales;*p;p++)	{
		SCXMLEstado *h=hijoP(hijos,*p);
		h->termina();
	}
	if (onexit) for (SCXMLEjecutable **p2=onexit;*p2;p2++) (*p2)->ejecutar(datos);
	delete[] actuales;
	actuales=NULL;
}

void SCXMLEstadoParalelo::entrar()	{
	estaCorriendo=true;
	if (onentry) for (SCXMLEjecutable **p=onentry;*p;p++) (*p)->ejecutar(datos);
	int c=0;
	for (SCXMLEstado **p=hijos;*p;p++) c++;
	actuales=new const char *[c+1];
	for (int i=0;i<c;i++)	{
		hijos[i]->entrar();
		actuales[i]=hijos[i]->getNombre();
	}
	actuales[c]=NULL;
}

char **separar(const char *lista)	{
	char *l=_strdup(lista);
	int cuantos=0;
	for (char *a=strtok(l,", ");a;a=strtok(NULL,", ")) cuantos++;
	free(l);
	char **res=new char*[cuantos+1];
	int i=0;
	l=_strdup(lista);
	for (char *a=strtok(l,", ");a;a=strtok(NULL,", ")) res[i++]=_strdup(a);
	res[cuantos]=NULL;
	delete l;
	return res;
}

void SCXMLEstadoParalelo::entrar(const char *estados)	{
	typedef struct	{
		SCXMLEstado *padre;
		char *hijos;
	}	strTMP;
	if (!strcmp(estados,nombre))	{
		entrar();
		return;
	}
	estaCorriendo=true;
	if (onentry) for (SCXMLEjecutable **p=onentry;*p;p++) (*p)->ejecutar(datos);
	char **sub=separar(estados);
	int numHijos=0;
	for (SCXMLEstado **p=hijos;*p;p++) numHijos++;
	strTMP *cadenas=new strTMP[numHijos];
	for (int i=0;i<numHijos;i++)	{
		cadenas[i].padre=hijos[i];
		cadenas[i].hijos=NULL;
	}
	for (char **s=sub;*s;s++)	{
		for (int i=0;i<numHijos;i++) if (cadenas[i].padre->soyYo(*s))	{
			if (!cadenas[i].hijos)	{
				cadenas[i].hijos=new char[strlen(*s)+1];
				strcpy(cadenas[i].hijos,*s);
			}	else	{
				int n=strlen(cadenas[i].hijos)+strlen(*s)+2;
				char *tmp=new char[n];
				strcpy(tmp,cadenas[i].hijos);
				strcat(tmp,",");
				strcat(tmp,*s);
				delete[] cadenas[i].hijos;
				cadenas[i].hijos=tmp;
			}
			break;
		}
		free(*s);
	}
	delete sub;
	int cuantos=0;
	for (int i=0;i<numHijos;i++) if (cadenas[i].hijos) cuantos++;
	actuales=new const char *[cuantos+1];
	int i=0;
	for (int j=0;j<numHijos;j++) if (cadenas[j].hijos)	{
		actuales[i++]=cadenas[j].padre->getNombre();
		cadenas[j].padre->entrar(cadenas[j].hijos);
		delete[] cadenas[j].hijos;
	}
	actuales[i]=NULL;
	delete[] cadenas;
}

bool SCXMLEstadoParalelo::transicion(const char *desde,const char *hacia,SCXMLTransition *t,SCXMLData *eventdata)	{
	if (!strcmp(desde,nombre))	{
		if (!strcmp(hacia,nombre))	{
			termina();
			t->ejecutar(eventdata);
			entrar();
			return false;
		}	else if (soyYo(hacia)) return false;
		else	{
			termina();
			return true;
		}
	}
	if (!soyYo(desde)) return false;
	SCXMLEstado *saliente=NULL;
	for (SCXMLEstado **p=hijos;*p;p++) if ((*p)->soyYo(desde))	{
		saliente=*p;
		break;
	}
	if (!saliente) throw runtime_error("Error desconocido gestionando los estados.");
	if (saliente->transicion(desde,hacia,t,eventdata))	{
		//NO SE PERMITEN TRANSICIONES ENTRE SUBESTADOS DE UN MISMO ESTADO PARALELO.
		termina(saliente->getNombre());
		return true;
	}	else return false;
}

list<TransicionActiva *> *SCXMLEstadoParalelo::evento(const char *n) const	{
	list<TransicionActiva *> *l=new list<TransicionActiva *>;
	if (!n)	{
		if (tInicial)	{
			TransicionActiva *t=new TransicionActiva;
			t->trans=tInicial;
			t->saliente=nombre;
			t->entrante=tInicial->getTarget();
			l->push_back(t);
		}
		for (SCXMLTransition **p=tSinEventos;*p;p++) if ((*p)->activar(NULL))	{
			TransicionActiva *t=new TransicionActiva;
			t->trans=*p;
			t->saliente=nombre;
			t->entrante=(*p)->getTarget();
			l->push_back(t);
			return l;
		}
	}	else for (SCXMLTransition **p=tConEventos;*p;p++) if ((*p)->activar(n))	{
		TransicionActiva *t=new TransicionActiva;
		t->trans=*p;
		t->saliente=nombre;
		t->entrante=(*p)->getTarget();
		l->push_back(t);
		return l;
	}
	//Añadir sólo si el destino es:
	//A) un subestado del origen
	//B) el mismo estado.
	//C) Un estado de fuera (en tal caso, retornar sólo esa transición y volver inmediatamente).
	//D) Inexistente (transición estándar)
	for (const char **p=actuales;*p;p++)	{
		list<TransicionActiva *> *lH=hijoP(hijos,*p)->evento(n);
		if (lH)	{
			while (lH->size()>0)	{
				TransicionActiva *t=lH->front();
				lH->pop_front();
				const char *dest=t->entrante;
				if (dest&&!soyYo(dest))	{
					for (list<TransicionActiva *>::iterator i=l->begin();i!=l->end();i++) delete *i;
					for (list<TransicionActiva *>::iterator i=lH->begin();i!=lH->end();i++) delete *i;
					l->clear();
					lH->clear();
					delete lH;
					l->push_back(t);
					return l;
				}	else l->push_back(t);
			}
			delete lH;
		}
	}
	return l;
}
