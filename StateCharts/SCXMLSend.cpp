#include "SCXMLSend.h"

ArbolXMLExpr **generarArbolXMLExpr(const NodoXML *n)	{
	ArbolXMLExpr **res=new ArbolXMLExpr *[n->cuantos+1];
	for (int i=0;i<n->cuantos;i++)	{
		res[i]->nombre=_strdup(n->hijos[i].nombre);
		if ((res[i]->esArbol=(n->hijos[i].cuantos>0))==true)
			res[i]->datos.hijos=generarArbolXMLExpr(n->hijos+i);
		else res[i]->datos.expr=SCXMLExpresion::crearExpresion(SCXML::atributo(n->hijos+i,"<value>"));
	}
	res[n->cuantos]=NULL;
	return res;
}

SCXMLSend::SCXMLSend(const NodoXML *n,list<EventoExterno *> *l)	{
	colaExterna=l;
	const char *cadena=SCXML::atributo(n,"event");	//Podría ser NULL
	if (cadena) nombre=_strdup(cadena);
	if (NULL!=(cadena=SCXML::atributo(n,"target")))
		target=_strdup(cadena);
	const char *d=SCXML::atributo(n,"delay");
	if (d)	{
		char *dd=_strdup(d);
		char *resto=NULL;
		delayMS=(float)strtod(dd,&resto);
		if (resto) if (!strcmp(resto,"s")) delayMS*=1000;
		free(dd);
	}	else delayMS=0;
	if (NULL!=(d=SCXML::atributo(n,"expr")))
	{
		tipoDatos=SCXMLSEND_EXPR;
		_eventData.expr=SCXMLExpresion::crearExpresion(d);
	}
	else if (NULL!=(d=SCXML::atributo(n,"namelist")))
	{
		tipoDatos=SCXMLSEND_NAMELIST;
		char *cad1=_strdup(d);
		int cuantos=0;
		for (char *tmp=strtok(cad1,", ");tmp;tmp=strtok(NULL,", ")) cuantos++;
		_eventData.namelist=new char*[cuantos+1];
		free(cad1);
		cad1=_strdup(d);
		int i=0;
		for (char *tmp=strtok(cad1,", ");i<cuantos;i++,tmp=strtok(NULL,", ")) _eventData.namelist[i]=_strdup(tmp);
		free(cad1);
	}	else if (n->cuantos)	{
		tipoDatos=SCXMLSEND_XML;
		_eventData.nodo=generarArbolXMLExpr(n);
	}	else tipoDatos=SCXMLSEND_SINDATOS;
}

void liberarArbolXMLExpr(ArbolXMLExpr **a)	{
	if (a)	{
		for (ArbolXMLExpr **p=a;*p;p++)	{
			free((*p)->nombre);
			if ((*p)->esArbol) liberarArbolXMLExpr((*p)->datos.hijos);
			else delete (*p)->datos.expr;
		}
		free(a);
	}
}

SCXMLSend::~SCXMLSend()	{
	char **p;
	switch (tipoDatos)	{
	case SCXMLSEND_EXPR:
		delete _eventData.expr;
		break;
	case SCXMLSEND_NAMELIST:
		for (p=_eventData.namelist;*p;p++) free(*p);
		free(_eventData.namelist);
		break;
	case SCXMLSEND_XML:
		liberarArbolXMLExpr(_eventData.nodo);
		break;
	}
	if (nombre) free(nombre);
	if (target) free(target);
}

SCXMLData **generarSCXMLData(ArbolXMLExpr **nodo,SCXMLDataModel *d)	{
	if (!nodo) return NULL;
	int c=0;
	for (ArbolXMLExpr **n=nodo;*n;n++) c++;
	SCXMLData **res=new SCXMLData *[c+1];
	for (int i=0;i<c;i++)	{
		res[i]->nombre=_strdup(nodo[i]->nombre);
		if (true==(res[i]->esArbol=nodo[i]->esArbol))
			res[i]->datos.ramas=generarSCXMLData(nodo[i]->datos.hijos,d);
		else res[i]->datos.cadena=nodo[i]->datos.expr->evaluarCadena(d);
	}
	res[c]=NULL;
	return res;
}

void SCXMLSend::ejecutar(SCXMLDataModel *d) const	{
	EventoExterno *e=new EventoExterno;
	e->nombre=nombre;
	e->target=target;
	e->procesados=0;
//	_LARGE_INTEGER ahora;
//	ahora.QuadPart=0;
//	_LARGE_INTEGER frec;
//	frec.QuadPart=0;
//	QueryPerformanceCounter(&ahora);
//	QueryPerformanceFrequency(&frec);
//	e->tstamp=ahora.QuadPart+(delayMS*(float)(frec.QuadPart)/1000);

	mrpt::system::TTimeStamp ahora = mrpt::system::now();
	e->tstamp= ahora + mrpt::system::secondsToTimestamp( delayMS*1e-3 );

	if (tipoDatos==SCXMLSEND_SINDATOS) e->_eventData=NULL;
	else	{
		e->_eventData=new SCXMLData;
		e->_eventData->nombre=_strdup("_eventData");
		if (tipoDatos==SCXMLSEND_EXPR)	{
			e->_eventData->esArbol=false;
			e->_eventData->datos.cadena=_eventData.expr->evaluarCadena(d);
		}	else	{
			e->_eventData->esArbol=true;
			if (tipoDatos==SCXMLSEND_NAMELIST)	{
				int c=0;
				char **p;
				for (p=_eventData.namelist;*p;p++) c++;
				e->_eventData->datos.ramas=new SCXMLData*[c+1];
				for (int i=0;i<c;i++)
					if (NULL!=(e->_eventData->datos.ramas[i]=SCXMLDataModel::duplicarNodoDeDatos(d->obtenerValor(p[i]))))
					{
					free(e->_eventData->datos.ramas[i]->nombre);
					e->_eventData->datos.ramas[i]->nombre=_strdup(p[i]);
				}	else throw runtime_error("No se ha encontrado el dato especificado en namelist.");
				e->_eventData->datos.ramas[c]=NULL;
			}	else e->_eventData->datos.ramas=generarSCXMLData(_eventData.nodo,d);
		}
	}
	/* Añadir con prioridad */
	list<EventoExterno *>::iterator i;
	for (i=colaExterna->begin();i!=colaExterna->end()&&(*i)->tstamp<=e->tstamp;i++);
	colaExterna->insert(i,e);	//La máquina de estados debe liberar después el evento.
}
