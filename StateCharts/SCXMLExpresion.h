#if !defined(AFX_SCXMLEXPRESION_H__89D1A67C_C35B_4709_9BE2_43DBA86443EE__INCLUDED_)
#define AFX_SCXMLEXPRESION_H__89D1A67C_C35B_4709_9BE2_43DBA86443EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXML.h"
#include "SCXMLDataModel.h"

class SCXMLExpresion	{
public:
	/* Métodos heredables de evaluación en forma de diversos tipos de datos de retorno. */
	virtual char *evaluarCadena(SCXMLDataModel *datos) const=0;
	virtual int evaluarEntero(SCXMLDataModel *datos) const;
	virtual float evaluarFlotante(SCXMLDataModel *datos) const;
	/* Esta clase abstracta no tiene constructor; en su lugar usa este método estático genérico. */
	static SCXMLExpresion *crearExpresion(const char *cadena);
};
#endif