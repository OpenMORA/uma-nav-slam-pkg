#include "SCXMLEvent.h"

SCXMLEvent::~SCXMLEvent()	{
	if (nombre) free(nombre);
}

void SCXMLEvent::ejecutar(SCXMLDataModel *d) const	{
	colaInterna->push_back(nombre);
	//printf("Executing event %s\n",nombre);
}

SCXMLEvent::SCXMLEvent(const char *n,list<const char *> *l)	{
	nombre=n?_strdup(n):NULL;
	colaInterna=l;
	//printf("Event created %s\n",n);
}
