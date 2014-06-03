#include "SCXMLCondicion.h"
#include "SCXMLCondicionSimple.h"
#include "SCXMLCondicionCompuesta.h"
#include <vector>
using namespace std;

#define SCXMLCOND_TOKEN_OPCOND 0
#define SCXMLCOND_TOKEN_OPEXPR 1
#define SCXMLCOND_TOKEN_SUBEXPR 2
#define SCXMLCOND_TOKEN_SUBCOND 3

typedef struct	{
	char tipo;
	union	{
		int opCond;
		int opExpr;
		SCXMLExpresion *subexpr;
		SCXMLCondicion *subcond;
	}	contenido;
}	SCXMLCondToken;

SCXMLCondicion *crearCondicionT(vector<SCXMLCondToken *> *v)	{
	//ACHTUNG! Código bestia
	bool op=false;
	vector<vector<SCXMLCondToken *>::iterator> opsCond;
	vector<vector<SCXMLCondToken *>::iterator> opsExpr;
	for (vector<SCXMLCondToken *>::iterator it=v->begin();it<v->end();it++)	{
		if (op!=((*it)->tipo<=SCXMLCOND_TOKEN_OPEXPR)) throw runtime_error("La secuencia de tokens es incorrecta.");
		if ((*it)->tipo==SCXMLCOND_TOKEN_OPCOND) opsCond.push_back(it);
		if ((*it)->tipo==SCXMLCOND_TOKEN_OPEXPR) opsExpr.push_back(it);
		op=!op;
	}
	if (v->size()%2!=1) throw runtime_error("Condición mal construida.");
	else if (v->size()==1)	{
		SCXMLCondToken *t=v->at(0);
		if (t->tipo!=SCXMLCOND_TOKEN_SUBCOND) throw runtime_error("Condición mal construida.");
		else return t->contenido.subcond;
	}	else if (opsCond.empty())	{
		if (v->size()!=3) throw runtime_error("Condición mal construida.");
		if ((v->at(0)->tipo!=SCXMLCOND_TOKEN_SUBEXPR)||(v->at(1)->tipo!=SCXMLCOND_TOKEN_OPEXPR)||(v->at(2)->tipo!=SCXMLCOND_TOKEN_SUBEXPR)) throw runtime_error("Condición mal construida.");
		opsExpr.clear();
		return new SCXMLCondicionSimple(v->at(0)->contenido.subexpr,v->at(2)->contenido.subexpr,v->at(1)->contenido.opExpr);
	}	else if (opsCond.size()==1)	{
		vector<SCXMLCondToken *> v1;
		vector<SCXMLCondToken *> v2;
		vector<SCXMLCondToken *>::iterator pos=opsCond.front();
		v1.assign(v->begin(),pos);
		v2.assign(pos+1,v->end());
		SCXMLCondicion *res=new SCXMLCondicionCompuesta(crearCondicionT(&v1),crearCondicionT(&v2),(*pos)->contenido.opCond);
		v1.clear();
		v2.clear();
		opsCond.clear();
		opsExpr.clear();
		return res;
	}	else	{
		vector<SCXMLCondToken *> v1;
		vector<vector<SCXMLCondToken *>::iterator>::iterator itIt=opsCond.begin();
		v1.assign(v->begin(),*itIt);
		SCXMLCondicion *tmp=crearCondicionT(&v1);
		v1.clear();
		do	{
			v1.assign(1+*itIt,*(1+itIt));
			tmp=new SCXMLCondicionCompuesta(tmp,crearCondicionT(&v1),(**itIt)->contenido.opCond);
			v1.clear();
			itIt++;
		}	while (itIt<opsCond.end());
		return tmp;
	}
}

SCXMLCondicion *SCXMLCondicion::crearCondicion(const char *cadena)	{
	vector<SCXMLCondToken *> v;
	SCXMLCondToken *t;
	for (const char *p=cadena;*p;)	{
		p+=strspn(p," \t\n");
		if (!*p) break;
		t=new SCXMLCondToken;
		if (*p=='(')	{
			const char *busq=p+1;
			int niv=1;
			while (niv)	{
				size_t len=strcspn(busq,"()");
				if (len==strlen(busq)) throw runtime_error("Condición mal construida.");
				busq+=len;
				if (*busq=='(') niv++;
				else niv--;
				busq++;
			}
			char *arg=new char[busq-p-1];
			strncpy(arg,p+1,busq-p-2);
			arg[busq-p-2]='\0';
			try	{
				SCXMLExpresion *e=SCXMLExpresion::crearExpresion(arg);
				t->tipo=SCXMLCOND_TOKEN_SUBEXPR;
				t->contenido.subexpr=e;
			}	catch (runtime_error)	{
				t->tipo=SCXMLCOND_TOKEN_SUBCOND;
				t->contenido.subcond=crearCondicion(arg);
			}
			delete arg;
			p=busq;
		}	else if (!strncmp(p,"==",2))	{
			t->tipo=SCXMLCOND_TOKEN_OPEXPR;
			t->contenido.opExpr=SCXMLCOND_IGUAL;
			p+=2;
		}	else if (!strncmp(p,"!=",2)||!strncmp(p,"<>",2))	{
			t->tipo=SCXMLCOND_TOKEN_OPEXPR;
			t->contenido.opExpr=SCXMLCOND_DISTINTO;
			p+=2;
		}	else if (!strncmp(p,"<=",2)||!strncmp(p,"=<",2))	{
			t->tipo=SCXMLCOND_TOKEN_OPEXPR;
			t->contenido.opExpr=SCXMLCOND_MENORIGUAL;
			p+=2;
		}	else if (!strncmp(p,">=",2)||!strncmp(p,"=>",2))	{
			t->tipo=SCXMLCOND_TOKEN_OPEXPR;
			t->contenido.opExpr=SCXMLCOND_MAYORIGUAL;
			p+=2;
		}	else if (*p=='>')	{
			t->tipo=SCXMLCOND_TOKEN_OPEXPR;
			t->contenido.opExpr=SCXMLCOND_MAYOR;
			p++;
		}	else if (*p=='<')	{
			t->tipo=SCXMLCOND_TOKEN_OPEXPR;
			t->contenido.opExpr=SCXMLCOND_MENOR;
			p++;
		}	else if (!strncmp(p,"&&",2))	{
			t->tipo=SCXMLCOND_TOKEN_OPCOND;
			t->contenido.opCond=SCXMLCOND_AND;
			p+=2;
		}	else if (!strncmp(p,"||",2))	{
			t->tipo=SCXMLCOND_TOKEN_OPCOND;
			t->contenido.opCond=SCXMLCOND_OR;
			p+=2;
		}	else	{
			size_t cuantos=strcspn(p,"=!<>&|");
			char *aux=new char[cuantos+1];
			strncpy(aux,p,cuantos);
			aux[cuantos]='\0';
			t->tipo=SCXMLCOND_TOKEN_SUBEXPR;
			t->contenido.subexpr=SCXMLExpresion::crearExpresion(aux);
			delete[] aux;
			p+=cuantos;
		}
		v.push_back(t);
	}
	SCXMLCondicion *c=crearCondicionT(&v);
	v.clear();
	return c;
}
