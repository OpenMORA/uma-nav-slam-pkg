#ifndef ManejadorSCXML_H
#define ManejadorSCXML_H

#if !defined(AFX_MANEJADORSCXML_H__0CCA6D09_1042_4638_8AC0_8D6385130841__INCLUDED_)
#define AFX_MANEJADORSCXML_H__0CCA6D09_1042_4638_8AC0_8D6385130841__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXML.h"
#include "SCXMLMaquinaDeEstados.h"
#include <list>

typedef struct	{
	SCXMLMaquinaDeEstados *maq;
	list<EventoExterno *> *salientes;
	list<EventoExterno *> *entrantes;
	const char *nombre;
}	ManejadorMaquina;

class ManejadorSCXML
{
private:
	unsigned int terminados;
	Rutinas *rut;

	char *carga(const char *nomArch);

	list<ManejadorMaquina *> *maqs;
	list<EventoExterno *> *global;
	//HANDLE mutex;
	mrpt::synch::CCriticalSection mutex;
	bool stop;
	bool parado;
	bool killthread;

public:
	ManejadorSCXML(Rutinas * r);
	virtual ~ManejadorSCXML();
	void anadirMaquina(const char *bloqueSCXML);
	void anadirMaquina(const NodoXML *n);
	void anadirEventoExterno(EventoExterno *e);
	void correr();	//ACHTUNG! Este método podría no volver nunca.
	void parar();
	void reanudar();
	void matar();
	void lee_fichero(std::string file);
	void NotifyNewTask(void *p,const char *label,const int uid,const char *comando,const int pri,const bool parallel);
	double Check4Vble(const char *name);

	void *pendientes;
};
#endif

#endif
