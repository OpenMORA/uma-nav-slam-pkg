#include "SCXMLLog.h"

SCXMLLog::~SCXMLLog()	{
	delete expr;
	free(label);
}

void SCXMLLog::ejecutar(SCXMLDataModel *d) const	{
	char *s=expr->evaluarCadena(d);
	static size_t idtask=0;

	TareaPendiente *t=new TareaPendiente;
	t->label=label;
	t->id=idtask;

	((list<TareaPendiente *> *)rut->pLista)->push_back(t);

	printf("Tarea enviada: \"%s\" (%s) - tid=%d.\n",label,s,t->id);
	rut->log(rut->pLista,label,t->id,s,0,true); //cga
	idtask++;


}

SCXMLLog::SCXMLLog(SCXMLExpresion *e,const char *lab,Rutinas *r)	{
	expr=e;
	label=lab?_strdup(lab):_strdup("");	//Se recomienda poner siempre una label.
	rut=r;
}
