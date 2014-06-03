#if !defined(AFX_SCXMLCONDICIONSIMPLE_H__4EEA8E97_54C9_4828_9993_4EF8FC2FDF8A__INCLUDED_)
#define AFX_SCXMLCONDICIONSIMPLE_H__4EEA8E97_54C9_4828_9993_4EF8FC2FDF8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLCondicion.h"

#define SCXMLCOND_DISTINTO 0
#define SCXMLCOND_IGUAL 1
#define SCXMLCOND_MENOR 2
#define SCXMLCOND_MENORIGUAL 3
#define SCXMLCOND_MAYOR 4
#define SCXMLCOND_MAYORIGUAL 5

class SCXMLCondicionSimple:public SCXMLCondicion	{
private:
	SCXMLExpresion *expr1;
	SCXMLExpresion *expr2;
	int operador;
public:
	SCXMLCondicionSimple(SCXMLExpresion *e1,SCXMLExpresion *e2,int tipo);
	SCXMLCondicionSimple(SCXMLExpresion *e1,SCXMLExpresion *e2,const char *cadena);
	virtual ~SCXMLCondicionSimple();
	virtual bool evaluar(SCXMLDataModel *d) const;
};

#endif