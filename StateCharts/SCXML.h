#if !defined(AFX_SCXML_H__91632FD1_C77E_4BB7_8A19_CD9942DBF141__INCLUDED_)
#define AFX_SCXML_H__91632FD1_C77E_4BB7_8A19_CD9942DBF141__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "xmlparse.h"
#include "xmltok.h"
#include <cstdlib>
#include <cstring>
#include <memory.h>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include "TypedefsSCXML.h"
using namespace std;

// JL: Fixes for MSVC stupid deprecations:
#ifndef MSC_VER
	#define _strdup strdup
#endif

typedef struct nodoxml	{
	char *nombre;
	char **attrs;
	nodoxml *hijos;
	nodoxml *padre;
	int cuantos;
}	NodoXML;
// Defines the maximum number of states you can manage. cga 29/06/2012
#define MAX_STATES 100
class SCXML	{
private:
	/* Funciones para comprobar si un nodo es correcto, dado su tipo.*/
	static bool esCorrectoSCXML(const NodoXML *n);
	static bool esCorrectoState(const NodoXML *n);
	static bool esCorrectoTransition(const NodoXML *n);
	static bool esCorrectoInitial(const NodoXML *n);
	static bool esCorrectoOnentry(const NodoXML *n);
	static bool esCorrectoOnexit(const NodoXML *n);
	static bool esCorrectoParallel(const NodoXML *n);
	static bool esCorrectoFinal(const NodoXML *n);
	static bool esCorrectoHistory(const NodoXML *n);
	static bool esCorrectoInvoke(const NodoXML *n);
	static bool esCorrectoParam(const NodoXML *n);
	static bool esCorrectoFinalize(const NodoXML *n);
	static bool esCorrectoContent(const NodoXML *n);
	static bool esCorrectoAnchor(const NodoXML *n);
	static bool esCorrectoDatamodel(const NodoXML *n);
	static bool esCorrectoData(const NodoXML *n);
	static bool esCorrectoAssign(const NodoXML *n);
	static bool esCorrectoEvent(const NodoXML *n);
	static bool esCorrectoSend(const NodoXML *n);
	static bool esCorrectoIf(const NodoXML *n);
	static bool esCorrectoElseif(const NodoXML *n);
	static bool esCorrectoElse(const NodoXML *n);
	static bool esCorrectoLog(const NodoXML *n);
	/* Función para comprobar si todos los hijos de un nodo son ejecutables correctamente formados */
	static bool esCorrectaSecuenciaEjecutable(const NodoXML *padre,bool esIf=false);
protected:
	NodoXML *Nglobal;
	NodoXML *Nactual;
	XML_Parser inicioConst();
public:
	/* Crea un manejador de SCXML a partir de un archivo. */
	SCXML(const char *arch);

	SCXML(const char *str,bool esArchivo);
	/* Destructor */
	virtual ~SCXML();
	/* Muestra el contenido de este nodo por pantalla (función de depuración) */
	static void mostrar(const NodoXML *n,int numtabs=0);
	/* Devuelve el atributo del nodo "n" con el nombre "nombre". Devuelve el puntero original, debe ser duplicado si es necesario. */
	static const char *atributo(const NodoXML *n,const char *nombre);
	/* Devuelve la estructura de datos del nodo. Devuelve el puntero original, debe ser duplicado si es necesario. */
	NodoXML *getGlobal() const;
	/* Comprueba exhaustivamente la corrección de este nodo. */
	bool sintaxisCorrecta() const;
	NodoXML *getActual() const;
	void setActual(NodoXML *);
	void setGlobal(NodoXML *);
};
#endif
