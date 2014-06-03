#include "SCXMLDataModel.h"

SCXMLData *SCXMLDataModel::parsearNodoDeDatos(NodoXML *n)	{
	SCXMLData *dat=new SCXMLData;
	dat->nombre=_strdup(n->nombre);
	const char *v=SCXML::atributo(n,"<valor>");
	if (v)	{
		dat->esArbol=false;
		dat->datos.cadena=_strdup(v);
	}	else	{
		dat->esArbol=true;
		dat->datos.ramas=new SCXMLData*;
		*dat->datos.ramas=NULL;
		for (int i=0;i<n->cuantos;i++) dat->datos.ramas=agregarDatos(dat->datos.ramas,parsearNodoDeDatos(n->hijos+i));
	}
	return dat;
}

SCXMLData *SCXMLDataModel::parsearData(NodoXML *n)	{
	const char *v=SCXML::atributo(n,"<valor>");
	SCXMLData *dat=new SCXMLData;
	dat->nombre=_strdup(SCXML::atributo(n,"name"));
	if (v)	{
		dat->esArbol=false;
		dat->datos.cadena=_strdup(v);
	}	else	{
		dat->esArbol=true;
		dat->datos.ramas=new SCXMLData*;
		*dat->datos.ramas=NULL;
		for (int i=0;i<n->cuantos;i++) dat->datos.ramas=agregarDatos(dat->datos.ramas,parsearNodoDeDatos(n->hijos+i));
	}
	return dat;
}

SCXMLDataModel::SCXMLDataModel(const NodoXML *n,SCXMLDataModel *p,Rutinas *func)	{
	padre=p;
	rut=func;
	datos=new SCXMLData*;
	*datos=NULL;

	if (n) for (int i=0;i<n->cuantos;i++) datos=agregarDatos(datos,parsearData(n->hijos+i));
}
SCXMLDataModel::SCXMLDataModel(const NodoXML *n,SCXMLDataModel *p)	{
	padre=p;

	datos=new SCXMLData*;
	*datos=NULL;

	if (n) for (int i=0;i<n->cuantos;i++) datos=agregarDatos(datos,parsearData(n->hijos+i));
}

SCXMLDataModel::~SCXMLDataModel()	{
	vaciar(datos);
}

SCXMLData *obtenerValorREC(char *busqueda,SCXMLData **ramas)	{
	char *cad1=strtok(busqueda,".");
	char *cad2=strtok(NULL,"");
	SCXMLData *res=NULL;
	for (SCXMLData **d=ramas;*d;d++) if (!strcmp((*d)->nombre,cad1))	{
		if (!cad2) res=*d;
		else if ((*d)->esArbol) res=obtenerValorREC(cad2,(*d)->datos.ramas);
		else res=NULL;
		break;
	}
	return res;
}


SCXMLData *SCXMLDataModel::obtenerValor(const char *s) const	{
	char *busqueda=_strdup(s);
	SCXMLData *res=obtenerValorREC(busqueda,datos);
	free(busqueda);
	if (padre&&!res)	{
		busqueda=_strdup(s);
		res=padre->obtenerValor(busqueda);
		free(busqueda);
	}
	return res;
}

void SCXMLDataModel::vaciar(SCXMLData **p1)	{
	for (SCXMLData **d=p1;*d;d++)	{
		vaciar(*d);
		free((*d)->nombre);
		free(*d);
	}
	free(p1);
}

void SCXMLDataModel::vaciar(SCXMLData *p1)	{
	if (p1->esArbol) vaciar(p1->datos.ramas);
	else free(p1->datos.cadena);
}

SCXMLData **agregarValorREC(char *busqueda,const char *cadena,SCXMLData **ramas)	{
	int c=0;
	char *cad1=strtok(busqueda,".");
	char *cad2=strtok(NULL,"");
	for (SCXMLData **d=ramas;*d;d++)	{
		c++;
		if (!strcmp((*d)->nombre,cad1))	{
			if (!cad2)	{
				SCXMLDataModel::vaciar(*d);
				(*d)->esArbol=false;
				(*d)->datos.cadena=_strdup(cadena);
			}	else	{
				if (!(*d)->esArbol)	{
					SCXMLDataModel::vaciar(*d);
					(*d)->esArbol=true;
					(*d)->datos.ramas=new SCXMLData*;
					*((*d)->datos.ramas)=NULL;
				}
				(*d)->datos.ramas=agregarValorREC(cad2,cadena,(*d)->datos.ramas);
			}
			return ramas;
		}
	}
	SCXMLData **nuevo=new SCXMLData*[c+2];
	for (int i=0;i<c;i++) nuevo[i]=ramas[i];
	free(ramas);
	nuevo[c]=new SCXMLData;
	nuevo[c]->nombre=_strdup(cad1);
	if (!cad2)	{
		nuevo[c]->esArbol=false;
		nuevo[c]->datos.cadena=_strdup(cadena);
	}	else	{
		nuevo[c]->esArbol=true;
		nuevo[c]->datos.ramas=new SCXMLData*;
		*(nuevo[c]->datos.ramas)=NULL;
		nuevo[c]->datos.ramas=agregarValorREC(cad2,cadena,nuevo[c]->datos.ramas);
	}
	nuevo[c+1]=NULL;
	return nuevo;
}

bool contieneRuta(SCXMLData **nodos,char *r)	{
	char *c1=strtok(r,".");
	char *c2=strtok(NULL,"");
	for (SCXMLData **p=nodos;*p;p++) if (!strcmp((*p)->nombre,c1))	{
		if (!c2) return true;
		else if ((*p)->esArbol) return contieneRuta((*p)->datos.ramas,c2);
		else return false;
	}
	return false;
}

void SCXMLDataModel::agregarValor(const char *dir,const char *cadena)	{
	char *busqueda=_strdup(dir);
	bool r=contieneRuta(datos,busqueda);
	free(busqueda);
	if (r)	{
		busqueda=_strdup(dir);
		datos=agregarValorREC(busqueda,cadena,datos);
		free(busqueda);
	}	else padre->agregarValor(dir,cadena);
}

void escribir(SCXMLData **que,const int donde)	{
	for (SCXMLData **p=que;*p;p++)	{
		for (int i=0;i<donde;i++) printf(" ");
		printf("%s:",(*p)->nombre);
		if ((*p)->esArbol)	{
			printf("\n");
			escribir((*p)->datos.ramas,donde+1);
		}	else printf("%s\n",(*p)->datos.cadena);
	}
}

void SCXMLDataModel::mostrar() const	{
	escribir(datos,0);
}

SCXMLData **SCXMLDataModel::duplicarArrayDeNodos(const SCXMLData **d)	{
	if (!d) return NULL;
	int c=0;
	for (const SCXMLData **p=d;*p;p++) c++;
	SCXMLData **res=new SCXMLData *[c+1];
	for (int i=0;i<c;i++) res[i]=duplicarNodoDeDatos(d[i]);
	res[c]=NULL;
	return res;
}

SCXMLData *SCXMLDataModel::duplicarNodoDeDatos(const SCXMLData *d)	{
	if (!d) return NULL;
	SCXMLData *res=new SCXMLData;
	res->nombre=d->nombre?_strdup(d->nombre):NULL;
	if ((res->esArbol=d->esArbol)==true)
		res->datos.ramas=duplicarArrayDeNodos((const struct nododatos **)d->datos.ramas);
	else res->datos.cadena=_strdup(d->datos.cadena);
	return res;
	
}

SCXMLDataModel::SCXMLDataModel(SCXMLData *dm,const char *nombre,SCXMLDataModel *p)	{
	if (dm)	{
		datos=new SCXMLData*[2];
		datos[0]=new SCXMLData;
		datos[0]->nombre=_strdup(nombre);
		datos[0]->esArbol=true;
		datos[0]->datos.ramas=new SCXMLData*[2];
		datos[0]->datos.ramas[0]=dm;
		datos[0]->datos.ramas[1]=NULL;
		datos[1]=NULL;
		padre=p;
	}	else	{
		datos=new SCXMLData*;
		*datos=NULL;
	}
}

SCXMLData **SCXMLDataModel::agregarDatos(SCXMLData **datos,SCXMLData *dato)	{
	int c=0;
	for (SCXMLData **p=datos;*p;p++) c++;
	SCXMLData **res=new SCXMLData*[c+2];
	for (int i=0;i<c;i++) res[i]=datos[i];
	res[c]=dato;
	res[c+1]=NULL;
	delete[] datos;
	return res;
}
