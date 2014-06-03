#include "SCXMLIf.h"
#include "SCXMLSend.h"
#include <vector>
using namespace std;

SCXMLIf::SCXMLIf(const NodoXML *n,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r)	{
	int nBloqs=2;
	list<int> pos;
	int i;
	for (i=0;i<n->cuantos;i++) if (!strcmp(n->hijos[i].nombre,"else")||!strcmp(n->hijos[i].nombre,"elseif"))	{
		nBloqs++;
		pos.push_back(i);
	}
	pos.push_back(i);
	bloques=new SCXMLBloqueIf*[nBloqs];
	bloques[0]=new SCXMLBloqueIf;
	bloques[0]->cond=SCXMLCondicion::crearCondicion(SCXML::atributo(n,"cond"));
	bloques[0]->sentencias=new SCXMLEjecutable*[pos.front()+1];
	for (i=0;i<pos.front();i++) bloques[0]->sentencias[i]=SCXMLEjecutable::crearEjecutable(n->hijos+i,lI,lE,r);
	bloques[0]->sentencias[pos.front()]=NULL;
	int indice;
	for (indice=1;;indice++)	{
		int inicial=pos.front();
		pos.pop_front();
		if (!pos.size()) break;
		int final=pos.front();
		bloques[indice]=new SCXMLBloqueIf;
		bloques[indice]->cond=(strcmp(n->hijos[inicial].nombre,"elseif"))?NULL:(SCXMLCondicion::crearCondicion(SCXML::atributo(n->hijos+i,"cond")));
		bloques[indice]->sentencias=new SCXMLEjecutable*[final-inicial];
		for (i=inicial+1;i<final;i++) bloques[indice]->sentencias[i-inicial-1]=SCXMLEjecutable::crearEjecutable(n->hijos+i,lI,lE,r);
		bloques[indice]->sentencias[final-inicial-1]=NULL;
	}
	bloques[indice+1]=NULL;
}

SCXMLIf::~SCXMLIf()	{
	for (SCXMLBloqueIf **p1=bloques;*p1;p1++)	{
		delete (*p1)->cond;
		for (SCXMLEjecutable **p2=(*p1)->sentencias;*p2;p2++) delete (*p2);
		delete[] (*p1)->sentencias;
	}
	delete[] bloques;
}

void SCXMLIf::ejecutar(SCXMLDataModel *d) const	{
	for (SCXMLBloqueIf **p1=bloques;*p1;p1++) if ((*p1)->cond?((*p1)->cond->evaluar(d)):true)	{
		for (SCXMLEjecutable **p2=(*p1)->sentencias;*p2;p2++) (*p2)->ejecutar(d);
		break;
	}
}