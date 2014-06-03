#include "SCXML.h"

char **duplicar(const char **arg)	{
	int c=0;
	for (const char **puntero=arg;*puntero;puntero++) c++;
	char **res=new char*[c+1];
	for (int i=0;i<c;i++) res[i]=_strdup(arg[i]);
	res[c]=NULL;
	return res;
}

void ponerAtributo(NodoXML *n,char *nombre,char *valor)	{
	int c=0;
	for (char **puntero=n->attrs;*puntero;puntero++) c++;
	char **res=new char*[c+3];
	for (int i=0;i<c;i++) res[i]=n->attrs[i];
	res[c]=nombre;
	res[c+1]=valor;
	res[c+2]=NULL;
	delete[] n->attrs;
	n->attrs=res;
}

void inicio(void *dat,const char *nombre,const char **attrs)	{
	SCXML *o=(SCXML *)dat;
	NodoXML *actual=o->getActual();
	NodoXML *global=o->getGlobal();
	if (!actual)	{
		actual=new NodoXML();
		global=actual;
		o->setGlobal(global);
		actual->nombre=_strdup(nombre);
		actual->attrs=duplicar(attrs);
		actual->hijos=NULL;
		actual->padre=NULL;
		actual->cuantos=0;
	}	else	{
		if (!actual->cuantos) {
			actual->hijos=new NodoXML[MAX_STATES];   //Caution: fix number of states you can manage!! 
		}
		else if (!(actual->cuantos%MAX_STATES))	{
			NodoXML *tmp=new NodoXML[actual->cuantos+MAX_STATES];
			for (int i=0;i<actual->cuantos;i++)	{
				tmp[i].nombre=actual->hijos[i].nombre;
				tmp[i].attrs=actual->hijos[i].attrs;
				tmp[i].hijos=actual->hijos[i].hijos;
				tmp[i].padre=actual;
			}
			delete[] actual->hijos;
			actual->hijos=tmp;
		}
		int ind=actual->cuantos;
		actual->hijos[ind].nombre=_strdup(nombre);
		actual->hijos[ind].attrs=duplicar(attrs);
		actual->hijos[ind].hijos=NULL;
		actual->hijos[ind].padre=actual;
		actual->hijos[ind].cuantos=0;
		actual->cuantos++;
		actual=actual->hijos+ind;
	}
	o->setActual(actual);
	
	
	
	
}

void fin(void *dat,const char *nombre)	{
	SCXML *o=(SCXML *)dat;
	o->setActual(o->getActual()->padre);
}

void pordefecto(void *dat,const char *cadena,int len)	{
	/*Para manejar los valores de datamodel, que no están dentro de una estructura SXCML.*/
	SCXML *o=(SCXML *)dat;
	if (cadena[0]==' '||cadena[0]=='<'||cadena[0]=='\t'||cadena[0]<='\n') return;
	char *cad=_strdup(cadena);
	char *cad2=_strdup(strtok(cad,"<>"));
	free(cad);
	ponerAtributo(o->getActual(),_strdup("<valor>"),cad2);
}

void liberar(NodoXML *nodo)	{
	free(nodo->nombre);
	for (char **p=nodo->attrs;*p;p++) free(*p);
	delete[] nodo->attrs;
	if (nodo->hijos)	{
		for (int i=0;i<nodo->cuantos;i++) liberar(nodo->hijos+i);
		delete[] nodo->hijos;
	}
}

XML_Parser SCXML::inicioConst()	{
	Nglobal=NULL;
	Nactual=NULL;
	XML_Parser parser=XML_ParserCreate("UTF-8");
	XML_SetUserData(parser,this);
	XML_SetElementHandler(parser,inicio,fin);
	XML_SetDefaultHandler(parser,pordefecto);
	return parser;
}

SCXML::SCXML(const char *arch)	{
	XML_Parser parser=inicioConst();
	ifstream fich(arch);
	if (fich.bad()) throw runtime_error("Error al abrir el archivo.");
	stringstream str; //=stringstream("");
	char buf[0x800];
	do	{
		fich.getline(buf,0x800);
		if (fich.bad()) throw runtime_error("Error al leer el archivo.");
		//str<<buf<<endl;
	}	while (!fich.eof());
	fich.close();
	string cstr=str.str();
	if (!XML_Parse(parser,cstr.c_str(),cstr.length(),1))	{
		liberar(Nglobal);
		delete Nglobal;
		throw runtime_error("Error parseando el contenido XML.");
	}
}

SCXML::SCXML(const char *instr,bool esArchivo)	{
	XML_Parser parser=inicioConst();
	const char *contenido;
	if (esArchivo)	{
		stringstream str; //=stringstream("");
		ifstream fich(instr);
		if (fich.bad()) throw runtime_error("Error al abrir el archivo.");
		char buf[0x800];
		do	{
			fich.getline(buf,0x800);
			if (fich.bad()) throw runtime_error("Error al leer el archivo.");
			//str<<buf<<endl;
		}	while (!fich.eof());
		contenido=str.str().c_str();
		fich.close();
	}	else contenido=instr;
	//printf("[%s]\n",contenido);
	if (!XML_Parse(parser,contenido,strlen(contenido),1))	{
		liberar(Nglobal);
		delete Nglobal;
		printf("Error parseando el fichero XML\n");
		throw runtime_error("Error parseando el contenido XML.");
	}

	mostrar(Nglobal);
	XML_ParserFree(parser);
}

SCXML::~SCXML()	{
	if (Nglobal)	{
		liberar(Nglobal);
		delete Nglobal;
	}
}

void SCXML::mostrar(const NodoXML *nodo,int numtabs)	{
	if (!nodo) return;
	for (int i=0;i<numtabs;i++) printf("  ");
	printf("%s ",nodo->nombre);
	int ind=0;
	while (nodo->attrs[ind])	{
		printf("[%s=%s] ",nodo->attrs[ind],nodo->attrs[ind+1]);
		ind++;
		ind++;
	}
	printf("\n");
	numtabs++;
	for (int i=0;i<nodo->cuantos;i++) mostrar(nodo->hijos+i,numtabs);
}

NodoXML *SCXML::getGlobal() const	{
	return Nglobal;
}

NodoXML *SCXML::getActual() const	{
	return Nactual;
}

void SCXML::setActual(NodoXML *a)	{
	Nactual=a;
}

void SCXML::setGlobal(NodoXML *g)	{
	Nglobal=g;
}

bool SCXML::sintaxisCorrecta() const	{
	if (!Nglobal) return false;
	if (strcmp(Nglobal->nombre,"scxml")) return false;
	return esCorrectoSCXML(Nglobal);
	return true;
}

bool SCXML::esCorrectoSCXML(const NodoXML *n)	{
	int initialstate=0;
	int xmlns=0;
	int version=0;
	int datamodel=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"initialstate")) initialstate++;
	else if (!strcmp(*p,"xmlns")) xmlns++;
	else if (!strcmp(*p,"version")) version++;
	else return false;	//Atributo incorrecto
	if (initialstate!=1||xmlns>1||version>1) return false;	//Cardinal de atributos
	//printf("Num states %d\n",n->cuantos);
	for (int i=0;i<n->cuantos;i++)	{
		char *tmp=n->hijos[i].nombre;
		if (!strcmp(tmp,"state"))	{
			if (!esCorrectoState(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"parallel"))	{
			if (!esCorrectoParallel(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"final"))	{
			if (!esCorrectoFinal(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"datamodel"))	{
			if (datamodel) return false;	//Cardinal de hijos
			if (!esCorrectoDatamodel(n->hijos+i)) return false;	//Corrección del hijo
			datamodel++;
		}	else if (!strcmp(tmp,"transition"))	{
			if (!esCorrectoTransition(n->hijos+i)) return false;	//Corrección del hijo
		}	else return false;	//Hijo incorrecto
	}
	return true;
}

bool SCXML::esCorrectoState(const NodoXML *n)	{
	int id=0;
	int src=0;
	int task=0;
	int onentry=0;
	int onexit=0;
	int initial=0;
	int estadosHijos=0;
	int datamodel=0;
	int invoke=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"id")) id++;
	else if (!strcmp(*p,"src")) src++;
	else if (!strcmp(*p,"task")) task++;
	else return false;	//Atributo incorrecto
	if (id!=1||src>1||task>1) return false;	//Cardinal de atributos
	for (int i=0;i<n->cuantos;i++)	{
		char *tmp=n->hijos[i].nombre;
		if (!strcmp(tmp,"onentry"))	{
			if (onentry) return false;	//Cardinal de hijos
			if (!esCorrectoOnentry(n->hijos+i)) return false;	//Corrección del hijo
			onentry++;
		}	else if (!strcmp(tmp,"onexit"))	{
			if (onexit) return false;	//Cardinal de hijos
			if (!esCorrectoOnexit(n->hijos+i)) return false;	//Corrección del hijo
			onexit++;
		}	else if (!strcmp(tmp,"transition"))	{
			if (!esCorrectoTransition(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"initial"))	{
			if (initial) return false;	//Cardinal de hijos
			if (!esCorrectoInitial(n->hijos+i)) return false;	//Corrección del hijo
			initial++;
		}	else if (!strcmp(tmp,"state"))	{
			estadosHijos++;
			if (!esCorrectoState(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"parallel"))	{
			estadosHijos++;
			if (!esCorrectoParallel(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"final"))	{
			estadosHijos++;
			if (!esCorrectoFinal(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"history"))	{
			if (!esCorrectoHistory(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"anchor"))	{
			if (!esCorrectoAnchor(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"datamodel"))	{
			if (datamodel) return false;	//Cardinal de hijos
			if (!esCorrectoDatamodel(n->hijos+i)) return false;	//Corrección del hijo
			datamodel++;
		}	else if (!strcmp(tmp,"invoke"))	{
			if (invoke) return false;	//Cardinal de hijos
			if (!esCorrectoInvoke(n->hijos+i)) return false;	//Corrección del hijo
			invoke++;
		}	else return false;	//Hijo incorrecto
	}
	if (invoke&&(estadosHijos>0)) return false;	//Hijos excluyentes
	if ((estadosHijos>0)&&!initial) return false;	//Hijo requerido
	return true;
}

bool SCXML::esCorrectoTransition(const NodoXML *n)	{
	int event=0;
	int cond=0;
	int target=0;
	int anchor=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"event")) event++;
	else if (!strcmp(*p,"cond")) cond++;
	else if (!strcmp(*p,"target")) target++;
	else if (!strcmp(*p,"anchor")) anchor++;
	else return false;	//Atributo incorrecto
	if (event>1||cond>1||target>1||anchor>1) return false;	//Cardinal de atributos
	if (!esCorrectaSecuenciaEjecutable(n)) return false;	//Secuencia ejecutable incorrecta
	return true;
}

bool SCXML::esCorrectoInitial(const NodoXML *n)	{
	int id=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"id")) id++;
	else return false;	//Atributo incorrecto
	if (id>1) return false;	//Cardinal de atributos
	if (n->cuantos!=1) return false;	//Debe tener siempre exactamente un hijo
	if (strcmp(n->hijos[0].nombre,"transition")) return false;	//El único hijo debe ser transition
	if (!esCorrectoTransition(n->hijos)) return false;	//Corrección del hijo
	return true;
}

bool SCXML::esCorrectoOnentry(const NodoXML *n)	{
	if (*(n->attrs)) return false;	//No puede tener atributos
	if (!esCorrectaSecuenciaEjecutable(n)) return false;	//Secuencia ejecutable incorrecta
	return true;
}

bool SCXML::esCorrectoOnexit(const NodoXML *n)	{
	if (*(n->attrs)) return false; //No puede tener atributos
	if (!esCorrectaSecuenciaEjecutable(n)) return false;	//Secuencia ejecutable incorrecta
	return true;
}

bool SCXML::esCorrectoParallel(const NodoXML *n)	{
	int id=0;
	int src=0;
	int task=0;
	int onentry=0;
	int onexit=0;
	int estadosHijos=0;
	int datamodel=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"id")) id++;
	else if (!strcmp(*p,"src")) src++;
	else if (!strcmp(*p,"task")) task++;
	else return false;	//Atributo incorrecto
	if (id!=1||src>1||task>1) return false;	//Cardinal de atributos
	for (int i=0;i<n->cuantos;i++)	{
		char *tmp=n->hijos[i].nombre;
		if (!strcmp(tmp,"onentry"))	{
			if (onentry) return false;	//Cardinal de hijos
			if (!esCorrectoOnentry(n->hijos+i)) return false;	//Corrección del hijo
			onentry++;
		}	else if (!strcmp(tmp,"onexit"))	{
			if (onexit) return false;	//Cardinal de hijos
			if (!esCorrectoOnexit(n->hijos+i)) return false;	//Corrección del hijo
			onexit++;
		}	else if (!strcmp(tmp,"state"))	{
			if (!esCorrectoState(n->hijos+i)) return false;	//Corrección del hijo
			estadosHijos++;
		}	else if (!strcmp(tmp,"parallel"))	{
			if (!esCorrectoParallel(n->hijos+i)) return false;	//Corrección del hijo
			estadosHijos++;
		}	else if (!strcmp(tmp,"history"))	{
			if (!esCorrectoHistory(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"anchor"))	{
			if (!esCorrectoAnchor(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"datamodel"))	{
			if (datamodel) return false;	//Cardinal de hijos
			if (!esCorrectoDatamodel(n->hijos+i)) return false;	//Corrección del hijo
			datamodel++;
		}	else if (!strcmp(tmp,"transition"))	{
			if (!esCorrectoTransition(n->hijos+i)) return false;	//Corrección del hijo
		}	else return false;	//Hijo incorrecto
	}
	if (!estadosHijos) return false;	//Cardinal de hijos
	return true;
}

bool SCXML::esCorrectoFinal(const NodoXML *n)	{
	int id=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"id")) id++;
	else return false;	//Atributo incorrecto
	if (id!=1) return false;	//Cardinal de atributos
	if (n->cuantos) return false;	//No debe tener hijos
	return true;
}

bool SCXML::esCorrectoHistory(const NodoXML *n)	{
	int id=0;
	int type=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"id")) id++;
	else if (!strcmp(*p,"type")) type++;
	else return false;	//Atributo incorrecto
	if (id!=1||type>1) return false;	//Cardinal de atributos
	if (n->cuantos!=1) return false;	//Debe tener siempre exactamente un hijo
	if (strcmp(n->hijos[0].nombre,"transition")) return false;	//El único hijo debe ser transition
	if (!esCorrectoTransition(n->hijos)) return false;	//Corrección del hijo
	return true;
}

bool SCXML::esCorrectoInvoke(const NodoXML *n)	{
	int id=0;
	int targettype=0;
	int src=0;
	int srcexpr=0;
	int finalize=0;
	int content=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"id")) id++;
	else if (!strcmp(*p,"targettype")) targettype++;
	else if (!strcmp(*p,"src")) src++;
	else if (!strcmp(*p,"srcexpr")) srcexpr++;
	else return false;	//Atributo incorrecto
	if (id!=1||targettype>1||src>1||srcexpr>1) return false;	//Cardinal de atributos
	if (src&&srcexpr) return false;	//Atributos excluyentes
	for (int i=0;i<n->cuantos;i++)	{
		char *tmp=n->hijos[i].nombre;
		if (!strcmp(tmp,"param"))	{
			if (!esCorrectoParam(n->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"finalize"))	{
			if (finalize) return false;	//Cardinal de hijos
			if (!esCorrectoFinalize(n->hijos+i)) return false;	//Corrección del hijo
			finalize++;
		}	else if (!strcmp(tmp,"content"))	{
			if (src+srcexpr+content) return false;	//Atributos e hijos excluyentes
			if (!esCorrectoContent(n->hijos+i)) return false;	//Corrección del hijo
			content++;
		}	else return false;	//Hijo incorrecto
	}
	return src+srcexpr+content==1;	//Cardinal de atributos e hijos
}

bool SCXML::esCorrectoParam(const NodoXML *n)	{
	int name=0;
	int expr=0;
	for (char** p=n->attrs;*p;p+=2) if (!strcmp(*p,"name")) name++;
	else if (!strcmp(*p,"expr")) expr++;
	else return false;	//Atributo incorrecto
	if (name!=1||expr>1) return false;	//Cardinal de atributos
	if (n->cuantos) return false;	//No debe tener hijos
	return true;
}

bool SCXML::esCorrectoFinalize(const NodoXML *n)	{
	if (*(n->attrs)) return false;	//No debe tener atributos
	if (!esCorrectaSecuenciaEjecutable(n)) return false;	//Corrección del hijo
	return true;
}

bool SCXML::esCorrectoContent(const NodoXML *n)	{
	if (*(n->attrs)) return false;	//No debe tener atributos
	return true;	//Por lo demás, es correcto siempre, a priori
}

bool SCXML::esCorrectoAnchor(const NodoXML *n)	{
	int type=0;
	int snapshot=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"type")) type++;
	else if (!strcmp(*p,"snapshot")) snapshot++;
	else return false;	//Atributo incorrecto
	if (type!=1||snapshot>1) return false;	//Cardinal de atributos
	if (n->cuantos) return false;	//No debe tener hijos
	return true;
}

bool SCXML::esCorrectoDatamodel(const NodoXML *n)	{
	int schema=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"schema")) schema++;
	else return false;	//Atributo incorrecto
	if (schema>1) return false;	//Cardinal de atributos
	for (int i=0;i<n->cuantos;i++) if (!strcmp(n->hijos[i].nombre,"data"))	{
		if (!esCorrectoData(n->hijos+i)) return false;	//Corrección del hijo
	}	else return false;	//Hijo incorrecto
	return true;
}

bool SCXML::esCorrectoData(const NodoXML *n)	{
	int name=0;
	int src=0;
	int expr=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"name")) name++;
	else if (!strcmp(*p,"src")) src++;
	else if (!strcmp(*p,"expr")) expr++;
	else if (!strcmp(*p,"<valor>")) continue;
	else return false;	//Atributo incorrecto
	if (name!=1||src>1||expr>1) return false;	//Cardinal de atributos
	return (src+expr+(n->cuantos?1:0))==1;	//Hijos y atributos excluyentes
}

bool SCXML::esCorrectoAssign(const NodoXML *n)	{
	int location=0;
	int src=0;
	int expr=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"location")) location++;
	else if (!strcmp(*p,"src")) src++;
	else if (!strcmp(*p,"expr")) expr++;
	else return false;	//Atributo incorrecto
	if (location!=1||src>1||expr>1) return false;	//Cardinal de atributos
	return (src+expr+(n->cuantos?1:0))==1;	//Hijos y atributos excluyentes
}

bool SCXML::esCorrectoEvent(const NodoXML *n)	{
	int name=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"name")) name++;
	else return false;	//Atributo incorrecto
	if (name!=1) return false;	//Cardinal de atributos
	if (n->cuantos) return false;	//No debe tener hijos
	return true;
}

bool SCXML::esCorrectoSend(const NodoXML *n)	{
	int event=0;
	int target=0;
	int targettype=0;
	int sendid=0;
	int delay=0;
	int namelist=0;
	int hints=0;
	int expr=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"event")) event++;
	else if (!strcmp(*p,"target")) target++;
	else if (!strcmp(*p,"targettype")) targettype++;
	else if (!strcmp(*p,"sendid")) sendid++;
	else if (!strcmp(*p,"delay")) delay++;
	else if (!strcmp(*p,"namelist")) namelist++;
	else if (!strcmp(*p,"hints")) hints++;
	else if (!strcmp(*p,"expr")) expr++;
	else if (!strncmp(*p,"xmlns:",6)) continue;
	else return false;	//Atributo incorrecto
	if (event>1||target>1||targettype>1||sendid>1||delay>1||namelist>1||hints>1||expr>1) return false;	//Cardinal de atributos
	if (expr+namelist+(n->cuantos?1:0)>1) return false;	//Hijos y atributos excluyentes
	return true;
}

bool SCXML::esCorrectoIf(const NodoXML *n)	{
	int cond=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"cond")) cond++;
	else return false;	//Atributo incorrecto
	if (cond!=1) return false;	//Cardinal de atributos
	if (!esCorrectaSecuenciaEjecutable(n,true)) return false;	//Corrección del hijo
	return true;
}

bool SCXML::esCorrectoElseif(const NodoXML *n)	{
	int cond=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"cond")) cond++;
	else return false;	//Atributo incorrecto
	if (cond!=1) return false;	//Cardinal de atributos
	if (n->cuantos) return false;	//No debe tener hijos
	return true;
}

bool SCXML::esCorrectoElse(const NodoXML *n)	{
	if (n->cuantos||*(n->attrs)) return false;	//No debe tener hijos ni atributos
	return true;
}

bool SCXML::esCorrectoLog(const NodoXML *n)	{
	int label=0;
	int expr=0;
	int level=0;
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,"label")) label++;
	else if (!strcmp(*p,"expr")) expr++;
	else if (!strcmp(*p,"level")) level++;
	else return false;
	if (n->cuantos) return false;	//No debe tener hijos
	return true;
}

bool SCXML::esCorrectaSecuenciaEjecutable(const NodoXML *padre,bool esIf)	{
	bool elseYa=false;
	for (int i=0;i<padre->cuantos;i++)	{
		char *tmp=padre->hijos[i].nombre;
		if (!strcmp(tmp,"assign"))	{
			if (!esCorrectoAssign(padre->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"event"))	{
			if (!esCorrectoEvent(padre->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"send"))	{
			if (!esCorrectoSend(padre->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"if"))	{
			if (!esCorrectoIf(padre->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"elseif"))	{
			if (elseYa) return false;	//No puede haber un elseif tras un else
			if (!esCorrectoElseif(padre->hijos+i)) return false;	//Corrección del hijo
		}	else if (!strcmp(tmp,"else"))	{
			if (elseYa) return false;	//No puede haber un else tras otro.
			if (!esCorrectoElse(padre->hijos+i)) return false;	//Corrección del hijo
			elseYa=true;
		}	else if (!strcmp(tmp,"log"))	{
			if (!esCorrectoLog(padre->hijos+i)) return false;	//Corrección del hijo
		}	else return false;	//Hijo incorrecto
	}
	return true;
}

const char *SCXML::atributo(const NodoXML *n,const char *nombre)	{
	for (char **p=n->attrs;*p;p+=2) if (!strcmp(*p,nombre)) return *(p+1);
	return NULL;
}
