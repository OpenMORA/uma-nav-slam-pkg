#if !defined(AFX_SCXMLEXPRESIONCADENA_H__576C52F0_F04B_48E5_9E86_17F63608B47D__INCLUDED_)
#define AFX_SCXMLEXPRESIONCADENA_H__576C52F0_F04B_48E5_9E86_17F63608B47D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SCXMLExpresion.h"

class SCXMLExpresionCadena : public SCXMLExpresion  
{
private:
	char *valor;
public:
	/* M�todo de evaluaci�n de cadena (los otros m�todos se heredan de SCXMLExpresion). */
	virtual char *evaluarCadena(SCXMLDataModel *datos) const;
	/* Constructor y destructor. */
	SCXMLExpresionCadena(char *cadena);
	virtual ~SCXMLExpresionCadena();
};
#endif