#include "SCXMLExpresion.h"
#include "SCXMLExpresionArit.h"
#include "SCXMLExpresionCadena.h"
#include "SCXMLExpresionConcat.h"
#include "SCXMLExpresionNumero.h"
#include "SCXMLExpresionVar.h"
#include "SCXMLExpresionRandom.h"
#include "SCXMLExpresionMOOSDBVble.h"
#include <vector>
using namespace std;

const unsigned char SCXMLEXPR_TOKEN_VAR=0;
const unsigned char SCXMLEXPR_TOKEN_CADENA=1;
const unsigned char SCXMLEXPR_TOKEN_NUMERO=2;
const unsigned char SCXMLEXPR_TOKEN_OPERADOR=3;
const unsigned char SCXMLEXPR_TOKEN_SUBEXPR=4;
const unsigned char SCXMLEXPR_TOKEN_MATHRAND=5;	//En un futuro, se podría usar simplemente "SCXMLEXPR_TOKEN_FUNCTION" o algo así.
const unsigned char SCXMLEXPR_TOKEN_MOOSVBLE=6;	//CGA

const char *const SCXML_RANDOM_FUNCTION_NAME="Math.Random()";
const char *const SCXML_MOOSVBLE_FUNCTION_NAME="MOOSDB.GetVariable(";

typedef struct	{
	char tipo;
	union	{
		char *nombrevar;
		char *cadena;
		float numero;
		char operador;
		SCXMLExpresion *subexpr;
	}	contenido;
}	SCXMLExprToken;

SCXMLExpresion *crearExpresionT(SCXMLExprToken *t)	{
	switch (t->tipo)	{
	case SCXMLEXPR_TOKEN_VAR:
		return new SCXMLExpresionVar(t->contenido.nombrevar);
	case SCXMLEXPR_TOKEN_CADENA:
		return new SCXMLExpresionCadena(t->contenido.cadena);
	case SCXMLEXPR_TOKEN_NUMERO:
		return new SCXMLExpresionNumero(t->contenido.numero);
	case SCXMLEXPR_TOKEN_SUBEXPR:
		return t->contenido.subexpr;
	case SCXMLEXPR_TOKEN_MATHRAND:
		return new SCXMLExpresionRandom();
	case SCXMLEXPR_TOKEN_MOOSVBLE:
		return new SCXMLExpresionMOOSDBVble(t->contenido.nombrevar);
	default:throw runtime_error("Token incorrecto.");
	}
}

SCXMLExpresion *crearExpresionT(const vector<SCXMLExprToken *> *v)	{
	//ACHTUNG! Código bastante abstruso a continuación
	if (v->size()==0) throw runtime_error("Expresión mal construida.");
	if (v->size()==1) return crearExpresionT((*v)[0]);
	bool op=false;
	vector<vector<SCXMLExprToken *>::const_iterator> alm;
	vector<vector<SCXMLExprToken *>::const_iterator> proddiv;
	vector<vector<SCXMLExprToken *>::const_iterator> sumres;
	vector<vector<SCXMLExprToken *>::const_iterator> pot;
	for (vector<SCXMLExprToken *>::const_iterator it=v->begin();it<v->end();it++)	{
		bool c=(*it)->tipo==SCXMLEXPR_TOKEN_OPERADOR;
		if (op!=c) throw runtime_error("La secuencia de tokens es incorrecta");
		if (c) switch ((*it)->contenido.operador)	{
			case '#':alm.push_back(it);break;
			case '*':
			case '/':
			case '%':proddiv.push_back(it);break;
			case '+':
			case '-':sumres.push_back(it);break;
			case '^':pot.push_back(it);break;
			default: throw runtime_error("Operador incorrecto");
		}
		op=!op;
	}
	vector<SCXMLExprToken *> v1;
	vector<SCXMLExprToken *> v2;
	vector<SCXMLExprToken *>::const_iterator pos;
	if (!alm.empty()) pos=alm.front();	//Da igual el orden
	else if (!proddiv.empty()) pos=proddiv.back();	//Asociativa, 2*3/4=(2*3)/4
	else if (!sumres.empty()) pos=sumres.back();	//Asociativa, 9-5+1=(9-5)+1
	else if (!pot.empty()) pos=pot.front();	//Asociativa, a^b^c=a^(b^c)
	else throw runtime_error("Expresión mal construida.");
	v1.assign(v->begin(),pos);
	v2.assign(pos+1,v->end());
	SCXMLExpresion *res=NULL;
	if (!alm.empty()) res=new SCXMLExpresionConcat(crearExpresionT(&v1),crearExpresionT(&v2));
	else res=new SCXMLExpresionArit(crearExpresionT(&v1),crearExpresionT(&v2),(*pos)->contenido.operador);
	v1.clear();
	v2.clear();
	alm.clear();
	proddiv.clear();
	sumres.clear();
	pot.clear();
	return res;
	
}

SCXMLExpresion *SCXMLExpresion::crearExpresion(const char *cadena)	{
	vector<SCXMLExprToken *> v;
	SCXMLExprToken *t;
	for (const char *p=cadena;*p;)	{
		p+=strspn(p," \t\n");
		if (!*p) break;
		if ('0'<=*p&&*p<='9')	{
			char *nuevo=NULL;
			t=new SCXMLExprToken;
			t->tipo=SCXMLEXPR_TOKEN_NUMERO;
			t->contenido.numero=(float)strtod(p,&nuevo);
			v.push_back(t);
			if (!nuevo) break;
			else p=nuevo;
		}	else if (*p=='\'')	{
			p++;
			const char *sig=strchr(p,'\'');
			if (!sig) throw runtime_error("Expresión mal construida (faltan las comillas de cierre).");
			t=new SCXMLExprToken;
			t->tipo=SCXMLEXPR_TOKEN_CADENA;
			t->contenido.cadena=new char[(sig-p)+1];
			strncpy(t->contenido.cadena,p,(sig-p));
			t->contenido.cadena[sig-p]='\0';
			v.push_back(t);
			p=sig+1;
		}	else if (*p=='(')	{
			const char *busq=p+1;
			int niv=1;
			while (niv)	{
				size_t len=strcspn(busq,"()");
				if (len==strlen(busq)) throw runtime_error("Expresión mal construida");
				busq+=len;
				if (*busq=='(') niv++;
				else niv--;
				busq++;
			}
			t=new SCXMLExprToken;
			t->tipo=SCXMLEXPR_TOKEN_SUBEXPR;
			char *arg=new char[busq-p-1];
			strncpy(arg,p+1,busq-p-2);
			arg[busq-p-2]='\0';
			t->contenido.subexpr=crearExpresion(arg);
			v.push_back(t);
			delete[] arg;
			p=busq;
		}	else if (*p==')') throw runtime_error("Expresión mal construida (falta el paréntesis de apertura).");
		else if (('A'<=*p&&*p<='Z')||('a'<=*p&&*p<='z')||(*p=='_'))	{
			//Código ad hoc: comprobar si esta cadena es la función "Math.random()"
			size_t SCXML_RANDOM_FUNCTION_LENGTH=strlen(SCXML_RANDOM_FUNCTION_NAME);
			size_t SCXML_MOOSVBLE_FUNCTION_LENGTH=strlen(SCXML_MOOSVBLE_FUNCTION_NAME);
			if ((strlen(p)>=SCXML_RANDOM_FUNCTION_LENGTH)&&(!strncmp(p,SCXML_RANDOM_FUNCTION_NAME,SCXML_RANDOM_FUNCTION_LENGTH)))	{
				t=new SCXMLExprToken;
				t->tipo=SCXMLEXPR_TOKEN_MATHRAND;
				v.push_back(t);
				p+=SCXML_RANDOM_FUNCTION_LENGTH;
				continue;
			}
			else if ((strlen(p)>=SCXML_MOOSVBLE_FUNCTION_LENGTH)&&(!strncmp(p,SCXML_MOOSVBLE_FUNCTION_NAME,SCXML_MOOSVBLE_FUNCTION_LENGTH)))	{
				//printf("MOOSVBLE expression recognized\n");
					t=new SCXMLExprToken;
					t->tipo=SCXMLEXPR_TOKEN_MOOSVBLE;
					v.push_back(t);
					p+=SCXML_MOOSVBLE_FUNCTION_LENGTH;
					//printf("$$%s$$\n",p);
					size_t len=strcspn(p,")");
					//printf("[%d]\n",len);
					char *vble=new char[len+1];
					strncpy(vble,p,len);
					//printf("%s\n",vble);
					p+=len+1;
					t->contenido.nombrevar=new char[len+1];
					strncpy(t->contenido.nombrevar,vble,len);
					t->contenido.nombrevar[len]='\0';
					//printf("%s\n",t->contenido.nombrevar);
					continue;
			

			}
			//Fin del código ad hoc
			size_t cuantos=strcspn(p," \t\n+-*/%^#");
			t=new SCXMLExprToken;
			t->tipo=SCXMLEXPR_TOKEN_VAR;
			t->contenido.nombrevar=new char[cuantos+1];
			strncpy(t->contenido.nombrevar,p,cuantos);
			t->contenido.nombrevar[cuantos]='\0';
			v.push_back(t);
			p+=cuantos;
		}	
		
		else switch (*p)	{
			case '+':
			case '-':
			case '*':
			case '/':
			case '^':
			case '#':
			case '%':
				t=new SCXMLExprToken;
				t->tipo=SCXMLEXPR_TOKEN_OPERADOR;
				t->contenido.operador=*p;
				v.push_back(t);
				p++;
				break;
			default:throw runtime_error("Expresión mal construida (carácter inesperado).");
		}
	}
	SCXMLExpresion *res=crearExpresionT(&v);
	for (vector<SCXMLExprToken *>::iterator it=v.begin();it<v.end();it++)	{
		switch ((*it)->tipo)	{
			case SCXMLEXPR_TOKEN_VAR:delete (*it)->contenido.nombrevar;break;
			case SCXMLEXPR_TOKEN_CADENA:delete (*it)->contenido.cadena;break;
		}
		delete *it;
	}
	return res;
}

int SCXMLExpresion::evaluarEntero(SCXMLDataModel *datos) const	{
	throw runtime_error("Esta expresión no puede ser evaluada como entero.");
}

float SCXMLExpresion::evaluarFlotante(SCXMLDataModel *datos) const	{
	throw runtime_error("Esta expresión no puede ser evaluada como flotante.");
}
