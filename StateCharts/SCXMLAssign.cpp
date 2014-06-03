#include "SCXMLAssign.h"

SCXMLAssign::SCXMLAssign(const NodoXML *n,list<const char *> *lI)	{
	if (SCXML::atributo(n,"expr"))	{
		location=_strdup(SCXML::atributo(n,"location"));
		contenido.esArbol=false;
		contenido.datos.expr=SCXMLExpresion::crearExpresion(SCXML::atributo(n,"expr"));
	}	else	{
		contenido.datos.hijos=new SCXMLData*;
		*contenido.datos.hijos=NULL;
		for (int i=0;i<n->cuantos;i++) contenido.datos.hijos=SCXMLDataModel::agregarDatos(contenido.datos.hijos,SCXMLDataModel::parsearNodoDeDatos(n->hijos+1));
	}
	notif=lI;
}

SCXMLAssign::~SCXMLAssign()	{
	free(location);
	if (contenido.esArbol) SCXMLDataModel::vaciar(contenido.datos.hijos);
	else delete contenido.datos.expr;
}

void agregarNodo(SCXMLDataModel *d,const char *dirbase,const SCXMLData *nodo)	{
	char *cad=new char[strlen(dirbase)+strlen(nodo->nombre)+2];
	strcpy(cad,dirbase);
	strcat(cad,".");
	strcat(cad,nodo->nombre);
	if (nodo->esArbol) for (SCXMLData **p=nodo->datos.ramas;*p;p++) agregarNodo(d,cad,*p);
	else d->agregarValor(cad,nodo->datos.cadena);
	delete cad;
}

void SCXMLAssign::ejecutar(SCXMLDataModel *d) const	{
	if (contenido.esArbol) for (SCXMLData **p=contenido.datos.hijos;*p;p++) agregarNodo(d,location,*p);
	else	{
		char *cadena=contenido.datos.expr->evaluarCadena(d);
		d->agregarValor(location,cadena);
		free(cadena);
	}
	notif->push_back(NULL);
}