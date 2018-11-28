#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <time.h>
#include <SFML/Network.hpp>

#include "GraficoSFML.h"
#include <SFML/Graphics.hpp>
#include "../XML/rapidxml.hpp"

/// --- TEMARIO DE PROCESOS, PIPES Y SIGNALS --- ///

///Librerias compartidas
#include <sys/wait.h>
#include <wait.h>
///Procesos, Signals
#include <unistd.h>
///Pipes
#include <fcntl.h>

/// --- TEMARIO DE MEMORIA COMP Y SEMAFOROS --- ///

///Librerias compartidas de SHM y SEM
#include <sys/types.h>
#include <sys/ipc.h>
///Memoria Compartida
#include <sys/shm.h>
///Semaforos
#include <sys/sem.h>

/// --- CONSTANTES --- ///
#define WINDOW_H 800
#define WINDOW_V 600
#define TITLE "Mi super practica 1.1"

#define MAX_BUFFER_RGB 9
#define MAX_BUFFER_S2 3

#define TIME_TODO 5

#define VACIO 0
#define ESPERA 1
#define OCUPADO 2
#define EMPEZANDO 3
#define COMIENDO 4
#define ACABADO 5

/// AUTORES: DAVID AULADELL, MAURIZIO CARLOTTA, ABRAHAM ARMAS CORDERO

