#include "ManejadorSCXML.h"

#include <mrpt/system.h> // sleep()


ManejadorSCXML::ManejadorSCXML(Rutinas * func)	{
	terminados=0;

	rut=new Rutinas;
	pendientes=(void *)new list<TareaPendiente *>;

	rut->pLista=pendientes;
	rut->log=func->log;
	rut->check=func->check;

	maqs=new list<ManejadorMaquina *>;
	global=new list<EventoExterno *>;
	parado=true;
	killthread = false;
	//mutex=CreateMutex(NULL,FALSE,"MutexManejadorSCXML");
}

ManejadorSCXML::~ManejadorSCXML()	{
	for (list<ManejadorMaquina *>::iterator i=maqs->begin();i!=maqs->end();i++)	{
		if (*i)	{
			if ((*i)->entrantes) delete (*i)->entrantes;
			if ((*i)->maq) delete (*i)->maq;
			delete *i;
		}
	}
	maqs->clear();
	delete maqs;
	for (list<EventoExterno *>::iterator j=global->begin();j!=global->end();j++) if (*j) delete *j;
	global->clear();
	delete global;
	//CloseHandle(mutex);
}

void ManejadorSCXML::correr()	{

	printf("-------------------------------\nStart Execution\n-------------------------------\n");
	stop=false;
	parado=false;
	//std::cout << "PASA A 1" << std::endl;
	//printf("Num maqs %d\n",maqs->size());
	for (list<ManejadorMaquina *>::iterator i=maqs->begin();i!=maqs->end();i++)	
	{
			(*i)->maq->entrar();
	}
	//std::cout << "PASA A 2" << std::endl;
	list<EventoExterno *> *procesados=new list<EventoExterno *>;
	//std::cout << "PASA A 3" << std::endl;
	bool init;
	while ((terminados<maqs->size()) && !killthread)	{
		if (stop) {mrpt::system::sleep(100);continue;}
		init=true;
		while (init)	{
			init=false;
			//printf("-->%d\n",procesados->size());
			for (list<EventoExterno *>::iterator ei=procesados->begin();ei!=procesados->end();ei++)
				if ((*ei)->procesados==maqs->size())	{
				delete *ei;
				procesados->erase(ei);
				init=true;
				break;
			}
		}

		// Código añadido el 19/05/08
		mrpt::system::sleep(100);
		// /Código

//		_LARGE_INTEGER ahora;
//		ahora.QuadPart=0;
//		QueryPerformanceCounter(&ahora);
		mrpt::system::TTimeStamp ahora = mrpt::system::now();
		
		//WaitForSingleObject(mutex,INFINITE);
		mutex.enter();

		//Revisa las colas de eventos externos lanzados por todas las máquinas y los añade...
		for (list<ManejadorMaquina *>::iterator i=maqs->begin();i!=maqs->end();i++)	{
			list<EventoExterno *> *lT=(*i)->entrantes;
			init=true;
			while (init)	{
				init=false;
				for (list<EventoExterno *>::iterator ei=lT->begin();ei!=lT->end();ei++)
					if ((*ei)->tstamp<=ahora)	{
						global->push_back(*ei);
						lT->erase(ei);
						init=true;
						break;
				}
			}
		}
		//ReleaseMutex(mutex);
		mutex.leave();

		if (global->size()<=0) continue;
		//WaitForSingleObject(mutex,INFINITE);
		mutex.enter();
		EventoExterno *e=global->front();
		global->pop_front();
		for (list<ManejadorMaquina *>::iterator i=maqs->begin();i!=maqs->end();i++) (*i)->salientes->push_back(e);
		procesados->push_back(e);
		if (e->nombre&&!strcmp(e->nombre,"top.DONE")) terminados++;
		mutex.leave();
		//ReleaseMutex(mutex);
	}
	for (list<EventoExterno *>::iterator ei=procesados->begin();ei!=procesados->end();ei++) delete *ei;
	procesados->clear();
	delete procesados;
	parado=true;
}


void ManejadorSCXML::anadirMaquina(const char *bloqueSCXML)	{

	SCXML *s=new SCXML(bloqueSCXML,false);
	
	anadirMaquina(s->getGlobal());
	delete s;
}

void ManejadorSCXML::anadirMaquina(const NodoXML *n)	{

	ManejadorMaquina *man=new ManejadorMaquina;
	man->entrantes=new list<EventoExterno *>;
	
	man->maq=new SCXMLMaquinaDeEstados(n,man->entrantes,rut,mutex);  //cga rut no sería necesario
	man->salientes=man->maq->colaEventos();
	man->nombre=NULL;
	maqs->push_back(man);
}

void ManejadorSCXML::anadirEventoExterno(EventoExterno *e)	{
	mrpt::synch::CCriticalSectionLocker lock ( &mutex );
	//WaitForSingleObject(mutex,INFINITE);
	global->push_back(e);
	//ReleaseMutex(mutex);
}

void ManejadorSCXML::parar()
{
	if (parado) 
		return;
	printf("parado\n");
	for (list<ManejadorMaquina *>::iterator i=maqs->begin();i!=maqs->end();i++)
	{
		if ((*i)->maq) 
			(*i)->maq->parar();	
	}
	stop = true;
	//while (!parado)
	//{
	//	mrpt::system::sleep(100);
	//}
	printf("fin parado\n");
}

void ManejadorSCXML::matar()	{
	//for (list<ManejadorMaquina *>::iterator i=maqs->begin();i!=maqs->end();i++) if ((*i)->maq) (*i)->maq->parar();
	killthread=true;
	parado = false;
	maqs->erase(maqs->begin(),maqs->end());
	//while (!parado)
//	{
//		mrpt::system::sleep(100);
//	}
}




void ManejadorSCXML::reanudar()	{
	//if (!parado) return;
	printf("reanudado\n");
	for (list<ManejadorMaquina *>::iterator i=maqs->begin();i!=maqs->end();i++) if ((*i)->maq) (*i)->maq->reanudar();
	stop=false;
	parado=false;
	printf("fin reanudado\n");
	
}

//-----------------------------------------------
// carga()
// Reads the content of the SCXML file into 
//-----------------------------------------------
char * ManejadorSCXML::carga(const char *nomArch)
{
	FILE *f=fopen(nomArch,"r");
	if (!f)
	{
		printf("File NOT found\n");
		return NULL;
	}

	char buf[0x800];
	stringstream ss;
	for (;;)
	{
		int c=fscanf(f,"%s",buf);
		if (c>0) ss<<buf<<' ';
		else break;
	}
	//Close the file and return content as a char*
	fclose(f);
	return _strdup(ss.str().c_str());
}


//----------------------------------------------
// lee_fichero()
// Loads a new SCXML file with the state charts
//----------------------------------------------
void ManejadorSCXML::lee_fichero(std::string file) 
{
	try	
	{
		killthread = false;
		printf("Objective file: [%s]\n",file.c_str());
		char *bloque = carga(file.c_str());				//read the SCXML file content into a char*	
	//	std::cout << bloque << std::endl;
		parar();
	//	printf("PASA 2\n");
		anadirMaquina(bloque);
	//	printf("PASA 3\n");
		free(bloque);
	//	printf("PASA 4\n");
		correr();
	//	printf("PASA 5\n");
	}
	catch (...)	
	{
		printf("Ha ocurrido un error al cargar el fichero.\n");
	}
}

