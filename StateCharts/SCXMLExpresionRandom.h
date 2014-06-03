#if !defined(AFX_SCXMLEXPRESIONRANDOM_H__8B82415E_9A83_45C2_9F69_17B583B4D67F__INCLUDED_)
#define AFX_SCXMLEXPRESIONRANDOM_H__8B82415E_9A83_45C2_9F69_17B583B4D67F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLExpresion.h"

class SCXMLExpresionRandom:public SCXMLExpresion	{
private:
	float valor;
public:
	/* Constructor y destructor. */
	SCXMLExpresionRandom();
	virtual ~SCXMLExpresionRandom();
	/* Métodos de evaluación. */
	virtual char *evaluarCadena(SCXMLDataModel *d) const;
	virtual int evaluarEntero(SCXMLDataModel *d) const;
	virtual float evaluarFlotante(SCXMLDataModel *d) const;
};
#endif