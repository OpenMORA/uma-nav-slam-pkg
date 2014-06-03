#if !defined(AFX_SCXMLSEND_H__D727CDC4_D90A_4608_9FA3_808C1E6B5D36__INCLUDED_)
#define AFX_SCXMLSEND_H__D727CDC4_D90A_4608_9FA3_808C1E6B5D36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEjecutable.h"
#include "SCXMLExpresion.h"
//#include <windows.h>
#include <list>
using namespace std;

#define SCXMLSEND_SINDATOS 0
#define SCXMLSEND_EXPR 1
#define SCXMLSEND_NAMELIST 2
#define SCXMLSEND_XML 3

typedef struct datosExpr	{
	char *nombre;
	bool esArbol;
	union	{
		SCXMLExpresion *expr;	//Si esArbol
		datosExpr **hijos;	//Si !esArbol
	}	datos;
}	ArbolXMLExpr;

class SCXMLSend:public SCXMLEjecutable	{
private:
	list<EventoExterno *> *colaExterna;
	char *nombre;
	char *target;
	float delayMS;
	int tipoDatos;
	union	{
		SCXMLExpresion *expr;
		char **namelist;
		ArbolXMLExpr **nodo;
	}	_eventData;
public:
	/* Constructor, recibe un puntero a la lista en la que introducirá eventos. */
	SCXMLSend(const NodoXML *n,list<EventoExterno *> *l);
	/* Destructor. */
	virtual ~SCXMLSend();
	/* Ejecución del comando. */
	virtual void ejecutar(SCXMLDataModel *d) const;
};
#endif
