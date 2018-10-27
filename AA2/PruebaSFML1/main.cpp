#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <time.h>

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
#define OCUPADO 1
#define COMIENDO 2

/// AUTORES: DAVID AULADELL, MAURIZIO CARLOTTA, ABRAHAM ARMAS CORDERO

/// -- DATOS -- ///
struct Data
{
    char TaburetesOcupados[3]; //0 = 1º taburete , 1= 2º taburete, 2 = 3º taburete
    sf::Color ClienteACargar; // if( this== null) { waitingForclients = true} else {waitingForClients = false; }

    public void SetDefault()
    {
        TaburetesOcupados[0](VACIO);
        TaburetesOcupados[1](VACIO);
        TaburetesOcupados[2](VACIO);
        ClienteACargar = null;
    }
    public bool RestauranteLleno()
    {
        for (int i = 0; i < 3; ++i)
        {
            if(TaburetesOcupados[i] == VACIO)
            {
                return false;
            }
        }
        return true;
    }
}
union semun
{
    int val;
    struct semid_ds* buf;
    unsigned short* array;
}
/// --- HIJO 1 ---

/// Esta variable puede pasar a ser local del hijo entero y pasarse por parametro a las funciones necesarias
std::vector<sf::Color> pedidos; //pedidos de los clientes

/// --- Control Clientes/Pedidos ---
const int RANDOM_TABURETES = 1; // control orden aparicion de los clientes *** 0 = ordenado de arriba abajo : 1 = orden aleatorio
const int RANDOM_PEDIDOS = 1;// control orden aparicion de los pedidos *** 0 ordenados segun XML : 1 = orden aleatorio

void TriggerAlarm(int param);
void CargarClienteSon(int param);//se ejecuta cuando el hijo 1 recibe un SIGALRM

/// --- HIJO 2 ---

/// Esta variable puede pasar a ser local del hijo entero y pasarse por parametro a las funciones necesarias
std::queue<int> TabureteComiendo = std::queue<int>();//se ejecuta cuando el hijo 2 recibe un SIGUSR2

void UnLoadWhatClient(int param);
void UnLoadClient(int param);

/// --- PADRE ---
GraficoSFML graficos;

void ClockAlarm(int param); //se triggerea cuando el padre le panda al hijo 0 un SIGUSR1
int FirstStoolFree();
void DrawClient(sf::Color color);
void VaciarMesa(int param);
void CargarClienteFath(int param);

int main()
{
    try
    {
        srand(time(NULL));
        ///crear memoria compartida con Data
        struct Data* shDatos;
        int shmID = shmget(IPC_PRIVATE, sizeof(struct Data), IPC_CREAT | 0666);
        if (shmID < 0) throw "La memoria compartida dio un problema";
        shDatos = (struct Data*)shmat(shmID,NULL,0);
        if (shDatos < 0) throw "El puntero a memoria compartida ha fallado";
        shDatos->SetDefault();

        int semID = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666); //dos semaforos uno para lso recursos del hijo 1 y otro para el hijo 2
        if (semID < 0) throw "Error al obtener semID"
        union semun arg;

        int dbg = semctl(semID,0,GETALL, arg);
        if(dbg < 0) throw "Error al objetener los semaforos para arg.array";

        arg.val = 1;
        dbg = semctl(semID,0,SETALL, arg);
        if(dbg < 0) throw "Error al hacer set de todos los semaforos";

        /// --- LLEGADA DE CLIENTES ---
        pid_t son1;
        if((son1 = fork()) == 0){
        /// Porque no directamente un SIGUSR1 y despues un sleep del tiempo que se necesite?
        signal(SIGUSR1, TriggerAlarm);
        signal(SIGALRM, CargarClienteSon);

        rapidxml::xml_document<> xmlFile;
        std::ifstream file("config.xml");
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        std::string content(buffer.str());
        xmlFile.parse<0>(&content[0]);

        rapidxml::xml_node<> *pRoot = xmlFile.first_node();

        /// Este parser sigue siendo necesario
        for(rapidxml::xml_node<> *pNode = pRoot->first_node();pNode; pNode = pNode->next_sibling())
        {
            sf::Color newPedido;

            for (rapidxml::xml_attribute<> *pAttr = pNode->first_attribute(); pAttr; pAttr = pAttr->next_attribute())
            {
                if(pAttr->name()[0] == 'R')
                    newPedido.r = std::atoi(pAttr->value());
                else if(pAttr->name()[0] == 'G')
                    newPedido.g = std::atoi(pAttr->value());
                else if(pAttr->name()[0] == 'B')
                    newPedido.b = std::atoi(pAttr->value());
            }

            pedidos.push_back(newPedido);
        }

        while(1){ pause(); }

        exit(0);

    }else {

    /// --- CLIENTES COMIENDO ---

        pid_t son2;
        if((son2 = fork()) == 0){
            signal(SIGALRM, UnLoadClient);
            signal(SIGUSR2, UnLoadWhatClient);

            while(1){ pause(); }

            exit(0);
        }
        else{
            sf::RenderWindow window(sf::VideoMode(WINDOW_H,WINDOW_V), TITLE);

            signal(SIGALRM,ClockAlarm);
            signal(SIGUSR1, CargarClienteFath);
            signal(SIGUSR2, VaciarMesa);
            alarm(1);
            //VARIABLES
            sf::Event event;

            bool mouseLeftButtPressed = false;
            bool mouseRightButtPressed = false;

            std::string buffer;

            while(window.isOpen())
            {
                ///EVENTOS
                while(window.pollEvent(event))
                {
                    if (event.type == sf::Event::Closed)
                    {
                        window.close();
                    }
                    if (event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Escape)
                        {
                            window.close();
                        }
                    }
                    if (event.type == sf::Event::MouseButtonPressed)
                    {
                        if (event.mouseButton.button == sf::Mouse::Left)
                        {
                            mouseLeftButtPressed = true;
                        }
                        else if (event.mouseButton.button == sf::Mouse::Right)
                        {
                            mouseRightButtPressed = true;
                        }
                    }
                }

                ///UPDATE
                {

                    //Respuesta Inputs
                    if(mouseLeftButtPressed)
                    {
                        graficos.MueveJugador(sf::Vector2f(event.mouseButton.x, event.mouseButton.y));

                            for(int i = 1; i < 4; i++)
                            {
                                if (graficos.aObjetosADibujar[graficos.aObjetosADibujar.size() - i].getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y)))
                                {
                                    graficos.CogeComida(graficos.aObjetosADibujar[graficos.aObjetosADibujar.size() - i].getFillColor());

                                }
                            }
                            for(int i = 0; i < 3; i++)
                            {
                                //Comprobamos si el jugador ha clickado sobre un cliente y tiene la comida que este necesita
                                if(graficos.aTaburetesADibujar[i].getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y)) && graficos.TabureteOcupado(i))
                                {
                                    if(graficos.manoDerecha.getFillColor() == graficos.aPedidosADibujar[i].getFillColor() || graficos.manoIzquierda.getFillColor() == graficos.aPedidosADibujar[i].getFillColor())
                                    {
                                        buffer = std::to_string(i);
                                        graficos.aTaburetesADibujar[i].setFillColor(TABURETE_COMIENDO);
                                        write(fdS2[1],buffer.c_str(),1);
                                        kill(son2,SIGUSR2);

                                        graficos.DejaComida(graficos.aPedidosADibujar[i].getFillColor());


                                    }

                                }
                            }
                            mouseLeftButtPressed = false;
                    }
                    else if(mouseRightButtPressed)
                    {
                        graficos.TiraComida();
                        mouseRightButtPressed = false;
                    }

                    //Comprobación mesa libre
                    if(waitingForClient && !graficos.RestauranteLLeno())
                    {
                        kill(son1, SIGUSR1);
                        /// podriamos hacer que esta variable estuviese en memoria compartida y que la leyese el hijo
                        waitingForClient = false;
                    }
                }

                ///DRAW
                {

                    window.clear(sf::Color(200,200,200,255));


                    for(size_t i = 0; i < graficos.aTextosADibujar.size(); i++)
                    {
                        window.draw(graficos.aTextosADibujar[i]);
                    }
                    for(size_t i = 0; i < graficos.aObjetosADibujar.size(); i++)
                    {
                        window.draw(graficos.aObjetosADibujar[i]);
                    }

                    for (size_t i =0 ; i< NUM_MESAS; i++)
                    {
                        window.draw(graficos.aTaburetesADibujar[i]);
                        window.draw(graficos.aPedidosADibujar[i]);
                    }

                    graficos.UpdateTimer();
                    graficos.UpdateClientCounter();
                    window.draw(graficos.jugador);
                    window.draw(graficos.manoIzquierda);
                    window.draw(graficos.manoDerecha);

                    window.display();

                }

                if (graficos.tiempoRestante <= 0 || graficos.numClientesRestantes == 0)
                window.close();
            }
            //esperar a todos los hijos
            //matarlos
            kill(son1, SIGKILL);
            kill(son2, SIGKILL);
            shmdt(shDatos);
            shmctl(shmID, IPC_RMID, NULL);
            //exit
            return 0;
       }
    }
    }
    catch (std::exception e){
        std::cout << e.what();
    }
}

/// --- HIJO 1 ---
void TriggerAlarm(int param)
{
    alarm(TIME_TODO);
}

void CargarClienteSon(int param)
{
    if (!pedidos.empty())
    {
        char* n = new char[MAX_BUFFER_RGB];
        std::string rgb;
        if(RANDOM_PEDIDOS == 0)
        {
            rgb = std::to_string(pedidos.front().r) +  std::to_string(pedidos.front().g) + std::to_string(pedidos.front().b);
            pedidos.erase(pedidos.begin());
        }
        else
        {
            int rnd = rand() % pedidos.size();
            rgb = std::to_string(pedidos[rnd].r) +  std::to_string(pedidos[rnd].g) + std::to_string(pedidos[rnd].b);
            pedidos.erase(pedidos.begin() +rnd);
        }
        /// no haria falta tener un parser con memoria compartida
        n = rgb.c_str();
        write(fdS1[1],n,MAX_BUFFER_RGB);
        kill(getppid(), SIGUSR1);

    }
}

/// --- HIJO 2 ---
void UnLoadWhatClient(int param)
{
    pid_t pid;
    char* buffer = new char[MAX_BUFFER_S2];
    size_t t = read(fdS2[0],buffer, MAX_BUFFER_S2);
    buffer[t] = '\0';

    int n = std::stoi(buffer);
    TabureteComiendo.push(n);

    if(pid = fork() == 0)
    {
        sleep(5);
        kill(getppid(), SIGALRM);
        exit(0);
    }

}

void UnLoadClient (int param)
{
    char* buffer = new char[MAX_BUFFER_S2];
    buffer = std::to_string(TabureteComiendo.front()).c_str();

    write(fdS2[1],buffer,1);

    TabureteComiendo.pop();
    kill(getppid(), SIGUSR2);
}

/// --- PADRE ---
void ClockAlarm(int param)
{
    graficos.tiempoRestante--;
    alarm(1);
}

int FirstStoolFree()
{
    for(size_t x = 0; x < 3; x++)
    {
        if(!graficos.stoolState[x])
        {
            return x;
        }
    }
    return -1;
}

void DrawClient(sf::Color color)
{
     //Llenamos el taburete en nuestra array
     if(!graficos.RestauranteLLeno())
    {
        //Llenamos el taburete en nuestra array

        if (RANDOM_TABURETES == 0)
        {
            int i = 0;

            while(i  < 4 && graficos.TabureteOcupado(i))
            {
                i++;
            }

            if (i < 3)
            {
                graficos.OcupaTaburete(i);
                graficos.aPedidosADibujar[i].setFillColor(color);
                waitingForClient = true;
            }
        }
        else
        {
            std::vector<int>  emptyChair = std::vector<int>();

            for(int i = 0; i < NUM_MESAS; i++)
            {
                if (!graficos.TabureteOcupado(i)) emptyChair.push_back(i);
            }

            int nuevoSitio = rand() % emptyChair.size();
            graficos.OcupaTaburete(emptyChair[nuevoSitio]);
            graficos.aPedidosADibujar[emptyChair[nuevoSitio]].setFillColor(color);
            waitingForClient = true;
        }

    }
}


void VaciarMesa(int param)
{
    char* buffer = new char[MAX_BUFFER_S2];

    size_t x = read(fdS2[0], buffer, 1);
    buffer[x] = '\0';

    int index = std::stoi(buffer);

    graficos.numClientesRestantes--;
    graficos.aTaburetesADibujar[index].setFillColor(TABURETE_VACIO);
    graficos.aPedidosADibujar[index].setFillColor(PEDIDO_VACIO);
    graficos.stoolState[index] = false;

}

void CargarClienteFath(int param){
//SIG1

    sf::Color auxCol = sf::Color::White;
    std::string aux;
    char * buffer = new char[MAX_BUFFER_RGB];
    int colInt[3];

    //Leemos los 9 numeros que componen el color de la comida
    size_t is = read(fdS1[0],buffer, MAX_BUFFER_RGB);

    int control = 0;
///no haria falta este parser
  for(size_t x {0}; x < 3; x++)
    {

        if(buffer[control] == '0')
        {
            aux = "0";
            control++;

        }
        else
        {
            aux = {buffer[control], buffer[control+1], buffer[control+2]};
            control += 3;
        }
          colInt[x] = std::stoi(aux);
    }

    auxCol = sf::Color(colInt[0], colInt[1], colInt[2], 255);
    DrawClient(auxCol);
}

