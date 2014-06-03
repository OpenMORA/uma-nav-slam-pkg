#if !defined(AFX_SCXMLASSIGN_H__B8F36C55_BD2C_4767_98D1_1BB3902BA02E__INCLUDED_)
#define AFX_SCXMLASSIGN_H__B8F36C55_BD2C_4767_98D1_1BB3902BA02E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLEjecutable.h"
#include "SCXMLExpresion.h"

class SCXMLAssign:public SCXMLEjecutable	{
private:
	char *location;
	struct	{
		bool esArbol;
		union	{
			SCXMLData **hijos;
			SCXMLExpresion *expr;
		}	datos;
	}	contenido;
	list<const char *> *notif;
public:
	/* Constructor, crea el objeto a partir de un nodo assign. */
	SCXMLAssign(const NodoXML *n,list<const char *> *lI);
	/* Destructor. */
	virtual ~SCXMLAssign();
	/* Código para ejecutar en caso de que la máquina de estados llegue a la sentencia assign. */
	virtual void ejecutar(SCXMLDataModel *d) const;
};
#endif
