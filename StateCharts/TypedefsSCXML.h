#ifndef _TYPEDEFS_SCXML_
#define _TYPEDEFS_SCXML_

typedef struct	{
	const char *label;
	int id;
}	TareaPendiente;

//typedef void (*FunLog)(void *,const char *,const int,const char *,const int,const bool);
/* Lista de parámetros de FunLog:
	1 (const char *): etiqueta.
	2 (const int): id de usuario.
	3 (const char *): comando (es la función la que lo parsea, separando comando y argumentos).
	4 (const int): prioridad (0 ya va bien).
	5 (const bool): paralelizable (true, normalmente, y que el planificador haga lo que vea).
	*/

//typedef void (*FunLog)(const int,const char *,const char *,const int,const bool);

typedef struct	{
	void (*log)(void *,const char *,const int,const char *,const int,const bool);
	double (*check)(const char*);
	void *pLista;
}	Rutinas;	//Lista de rutinas externas

class ManejadorSCXML;
#endif