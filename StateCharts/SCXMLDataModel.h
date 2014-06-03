#if !defined(AFX_SCXMLDATAMODEL_H__B2A9D4EC_D603_4C4A_8400_8288A7C74879__INCLUDED_)
#define AFX_SCXMLDATAMODEL_H__B2A9D4EC_D603_4C4A_8400_8288A7C74879__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXML.h"
#include <string>
using namespace std;

typedef struct nododatos	{
	char *nombre;
	bool esArbol;
	union	{
		char *cadena;	//Sólo si !esArbol
		nododatos **ramas;	//Sólo si esArbol.
	}	datos;
}	SCXMLData;

class SCXMLDataModel	{
private:
	//Apunta al datamodel inmediatamente superior; NULL para el datamodel del estado top.
	SCXMLDataModel *padre;
	/* Puntero a datos, almacenados del mismo modo que argv: si hay 3 entradas, entonces datos es un array de 4 entradas, 
		la última de las cuales apunta a NULL.*/
	SCXMLData **datos;

	
public:
	Rutinas *rut;

	/* Libera completamente un puntero SCXMLData **. */
	static void vaciar(SCXMLData **p1);
	/* Vacía el contenido de un puntero SCXMLData *, liberando toda la memoria pero no la del propio puntero. */
	static void vaciar(SCXMLData *p1);
	/* Constructor, crea el DataModel a partir de su nodo XML. Puede recibir también un puntero al nodo XML superior. */
	SCXMLDataModel(const NodoXML *n,SCXMLDataModel *p=NULL);
	/* cga */
	SCXMLDataModel(const NodoXML *n,SCXMLDataModel *p,Rutinas * func);
	/* Constructor rápido pensado para las transiciones. */
	SCXMLDataModel(SCXMLData *dm,const char *nombre,SCXMLDataModel *p);
	/* Destructor. */
	virtual ~SCXMLDataModel();
	/* Obtiene el puntero al nodo de datos, dada una cadena con la dirección de la variable. Los datos de este puntero
	   pueden ser modificados por el programa llamante. */
	SCXMLData *obtenerValor(const char *s) const;
	/* Añade un valor de cadena a este DataModel. Si el atributo no existe, lo crea. */
	void agregarValor(const char *dir,const char *cadena);
	/* Muestra por pantalla el contenido de este DataModel. */
	void mostrar() const;
	/* Funciones auxiliares para manejar nodos de datos. */
	static SCXMLData *parsearData(NodoXML *n);
	static SCXMLData *parsearNodoDeDatos(NodoXML *n);
	static SCXMLData **duplicarArrayDeNodos(const SCXMLData **d);
	static SCXMLData *duplicarNodoDeDatos(const SCXMLData *d);
	static SCXMLData **agregarDatos(SCXMLData **datos,SCXMLData *dato);
};
#endif