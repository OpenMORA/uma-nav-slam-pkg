#include "SCXMLEjecutable.h"
#include "SCXMLAssign.h"
#include "SCXMLEvent.h"
#include "SCXMLSend.h"
#include "SCXMLIf.h"
#include "SCXMLLog.h"

SCXMLEjecutable *SCXMLEjecutable::crearEjecutable(const NodoXML *n,list<const char *> *lI,list<EventoExterno *> *lE,Rutinas *r)	{
	if (!strcmp(n->nombre,"assign")) return new SCXMLAssign(n,lI);
	else if (!strcmp(n->nombre,"event")) return new SCXMLEvent(SCXML::atributo(n,"name"),lI);
	else if (!strcmp(n->nombre,"send")) return new SCXMLSend(n,lE);
	else if (!strcmp(n->nombre,"if")) return new SCXMLIf(n,lI,lE,r);
	else if (!strcmp(n->nombre,"log")) return new SCXMLLog(SCXMLExpresion::crearExpresion(SCXML::atributo(n,"expr")),SCXML::atributo(n,"label"),r);
	else throw runtime_error("Sentencia ejecutable de tipo desconocido.");
}
