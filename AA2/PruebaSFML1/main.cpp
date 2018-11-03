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
#define ESPERA 1
#define OCUPADO 2
#define EMPEZANDO 3
#define COMIENDO 4
#define ACABADO 5

/// AUTORES: DAVID AULADELL, MAURIZIO CARLOTTA, ABRAHAM ARMAS CORDERO

/// -- DATOS -- ///
struct Data
{
private:
    char tabureteState[3];
    sf::Color colorNuevoPedido;

public:
    void SetDefault() //un setter a default de todos los atributos
    {
        tabureteState[0] = VACIO;
        tabureteState[1] = VACIO;
        tabureteState[2] = VACIO;
        colorNuevoPedido = sf::Color().Black;
    }

    bool RestauranteLleno(int &semID, struct sembuf *tmpSemBuf) //retorna false si hay taburetes vacios
    {
        for (int i = 0; i < 3; ++i)
        {
            tmpSemBuf->sem_op = -1;
            semop(semID, tmpSemBuf, 1);

            if(tabureteState[i] == VACIO)
            {
                tmpSemBuf->sem_op = 1;
                semop(semID, tmpSemBuf, 1);
                return false;
            }
            //si retorna fasle aqui no ahce signal
            tmpSemBuf->sem_op = 1;
            semop(semID, tmpSemBuf, 1);
        }

        return true;
    }

    bool AlguienEmpezando(int &semID, struct sembuf *tmpSemBuf) //retorna true si alguien esta empezando se usa para evitar usar la funcion de descarga de clientes innecesariamente
    {
        for (int i = 0; i < 3; ++i)
        {
            tmpSemBuf->sem_op = -1;
            semop(semID, tmpSemBuf, 1);

            if(tabureteState[i] == EMPEZANDO)
            {
                tmpSemBuf->sem_op = 1;
                semop(semID, tmpSemBuf, 1);
                return true;
            }

            tmpSemBuf->sem_op = 1;
            semop(semID, tmpSemBuf, 1);
        }
        return false;
    }

    bool AlguienSatisfecho(int &semID, struct sembuf *tmpSemBuf)
    {
        for (int i = 0; i < 3; ++i)
        {
            tmpSemBuf->sem_op = -1;
            semop(semID, tmpSemBuf, 1);

            if(tabureteState[i] == ACABADO)
            {
                tmpSemBuf->sem_op = 1;
                semop(semID, tmpSemBuf, 1);
                return true;
            }

            tmpSemBuf->sem_op = 1;
            semop(semID, tmpSemBuf, 1);
        }
        return false;
    }

    void SetNewClientColor(sf::Color color, int &SemID, struct sembuf *tmpSemBuf)
    {

        tmpSemBuf->sem_op = -1;

        int i = semop(SemID, tmpSemBuf, 1);
        if(i < 0 ) std::cout << "Soy un bug del Wait de SetClient" << std::endl;

        //ZONA CRITICA
        colorNuevoPedido = color;

        tmpSemBuf->sem_op = 1;
        i = semop(SemID, tmpSemBuf, 1);
        if(i < 0 ) std::cout << "Soy un bug del Signal de SetClient" << std::endl;
    }

    void SetTaburetes(int Index, int Estado, int &SemID, struct sembuf *tmpSemBuf)
    {
        tmpSemBuf->sem_op = -1;
        int i = semop(SemID, tmpSemBuf, 1);
        if(i < 0 ) std::cout << "Soy un bug del Wait SetTaburete" << std::endl;
        //ZONA CRITICA
        tabureteState[Index] = Estado;

        tmpSemBuf->sem_op = 1;
        i = semop(SemID, tmpSemBuf, 1);
        if(i < 0 ) std::cout << "Soy un bug del Signal SetTaburete" << std::endl;


    }

    sf::Color GetNewClientColor(int semID, sembuf* sBuf)
    {
        sBuf->sem_op = -1;
        semop(semID, sBuf, 1);

        sf::Color aux(colorNuevoPedido);

        sBuf->sem_op = 1;
        semop(semID, sBuf, 1);
        return aux;

    }

    char GetTabureteState(int index, int semID, sembuf* sBuf)
    {
        sBuf->sem_op = -1;
        semop(semID, sBuf, 1);

        char aux(tabureteState[index]);

        sBuf->sem_op = 1;
        semop(semID, sBuf, 1);

        return aux;
    }

    int GetTabureteVacio(int semID, sembuf* sBuf)
    {
        for(int i = 0;i < 3; i++)
        {
            sBuf->sem_op = -1;
            semop(semID, sBuf, 1);

            if (tabureteState[i] == VACIO)
            {
                sBuf->sem_op = 1;
                semop(semID, sBuf, 1);
                return i;
            }
            sBuf->sem_op = 1;
            semop(semID, sBuf, 1);
        }
        return -1;
    }

    int GetClienteSatisfecho(int semID,sembuf* tmpSemBuf)
    {
        for(int i = 0;i < 3; i++)
        {
        tmpSemBuf->sem_op = -1;
        semop(semID, tmpSemBuf, 1);
            if (tabureteState[i] == ACABADO)
            {
                tmpSemBuf->sem_op = 1;
                semop(semID, tmpSemBuf, 1);
                return i;
            }
        tmpSemBuf->sem_op = 1;
        semop(semID, tmpSemBuf, 1);
        }
        return -1;
    }
};

union semun
{
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

/// --- Control Clientes/Pedidos ---
const int RANDOM_TABURETES = 1; // control orden aparicion de los clientes *** 0 = ordenado de arriba abajo : 1 = orden aleatorio
const int RANDOM_PEDIDOS = 1;// control orden aparicion de los pedidos *** 0 ordenados segun XML : 1 = orden aleatorio

/// --- HIJO 1 ---
std::vector<sf::Color> LoadXML();
sf::Color GetNewClientColor(std::vector<sf::Color> &pedidos);

/// --- HIJO 2 ---
void UnLoadWhatClient(struct Data*, int, sembuf*);
/// --- PADRE ---
int DrawNewClient(sf::Color color , Data *shData, int semId, struct sembuf *tempSemBuf);
void ClockAlarm(int param); //se triggerea cuando el padre le panda al hijo 0 un SIGUSR1
GraficoSFML graficos;

int main()
{
    try
    {
        srand(time(NULL));
        /// -- Crear Memoria Compartida
        struct Data* shDatos;
        int shmID = shmget(IPC_PRIVATE, sizeof(struct Data), IPC_CREAT | 0666);
        if (shmID < 0) throw "La memoria compartida dio un problema";

        shDatos = (struct Data*)shmat(shmID,NULL,0);
        if (shDatos < 0) throw "El puntero a memoria compartida ha fallado";
        shDatos->SetDefault();

        /// -- Crear Semaforos --
        int semID = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666); //dos semaforos uno para lso recursos del hijo 1 y otro para el hijo 2
        if (semID < 0) throw "Error al obtener semID";

        union semun arg;
        arg.val = 1;

        int dbg = semctl(semID,0,SETVAL, arg);
        if(dbg < 0) throw "Error al hacer set de todos los semaforos";

        struct sembuf *tmpSemBuf = new sembuf();
        tmpSemBuf->sem_num = 0;
        tmpSemBuf->sem_flg = SEM_UNDO;

        /// --- LLEGADA DE CLIENTES ---
        pid_t son1;
        if((son1 = fork()) == 0){
        std::vector<sf::Color> pedidos = LoadXML(); //pedidos de los clientes

            for (int i = 0; i < NUM_MESAS; i++)
            {
                shDatos->SetTaburetes(i,ESPERA,semID,tmpSemBuf);
                sf::Color nuevoColor = GetNewClientColor(pedidos);

                /// Este primer bucle hace 3 hijos con un sleep de 5 10 y 15 segundos
                /// despues los sleeps ponen el color dentro de la memoria compartida para que el padre lo lea
                if (fork() == 0) /// Bucle inicial.
                {
                    sleep(TIME_TODO*(i+1));
                    shDatos->SetNewClientColor(nuevoColor,semID,tmpSemBuf);

                    exit(3);
                }
            }


            while(1)
            {

                //si el restaurante no esta lleno el hijo empieza a enviar un cliente
                if (!shDatos->RestauranteLleno(semID,tmpSemBuf) && pedidos.size() > 0)
                {
                    shDatos->SetTaburetes(shDatos->GetTabureteVacio(semID,tmpSemBuf),ESPERA,semID,tmpSemBuf);
                    sf::Color nuevoColor= GetNewClientColor(pedidos);

                    if (fork() == 0)
                    {
                        sleep(TIME_TODO);
                        shDatos->SetNewClientColor(nuevoColor,semID,tmpSemBuf);
                        exit(0);
                    }
                }
            }
            exit(0);

        }

        else {
        /// --- CLIENTES COMIENDO ---
            pid_t son2;
            if((son2 = fork()) == 0)
            {
                while(1)
                {
                    if(shDatos->AlguienEmpezando(semID,tmpSemBuf))
                    {
                        UnLoadWhatClient(shDatos, semID, tmpSemBuf);
                    }
                }
            }
            else
            {
             /// Padre
                sf::RenderWindow window(sf::VideoMode(WINDOW_H,WINDOW_V), TITLE);


                ///control de tiempo
                graficos.tiempoRestante--;
                signal(SIGALRM,ClockAlarm);

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
                                            if(shDatos->GetTabureteState(i,semID,tmpSemBuf) == OCUPADO)
                                            {
                                                shDatos->SetTaburetes(i, EMPEZANDO, semID, tmpSemBuf);
                                                graficos.aTaburetesADibujar[i].setFillColor(TABURETE_COMIENDO);
                                                graficos.DejaComida(graficos.aPedidosADibujar[i].getFillColor());
                                            }
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

                        ///SI hay alguien satisfecho
                        if(shDatos->AlguienSatisfecho(semID,tmpSemBuf))
                        {
                            //Cojemos al que esta satisfecho
                            int i = shDatos->GetClienteSatisfecho(semID,tmpSemBuf);
                            graficos.aTaburetesADibujar[i].setFillColor(TABURETE_VACIO);
                            graficos.aPedidosADibujar[i].setFillColor(sf::Color().Black);
                            shDatos->SetTaburetes(i,VACIO,semID,tmpSemBuf);
                            graficos.numClientesRestantes --;

                            if(graficos.numClientesRestantes == 0)
                            {
                                std::cout << "All clients are satisfied, Good Job , Bye!" << std::endl;
                                sleep(2);
                                window.close();
                            }
                        }

                        ///entra cliente
                         if (shDatos->GetNewClientColor(semID,tmpSemBuf) != sf::Color().Black)
                        {
                            shDatos->SetTaburetes(DrawNewClient(shDatos->GetNewClientColor(semID,tmpSemBuf),shDatos,semID, tmpSemBuf), OCUPADO, semID,tmpSemBuf);
                            shDatos->SetNewClientColor(sf::Color().Black,semID,tmpSemBuf);
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

                    if (graficos.tiempoRestante <= 0)
                    {
                        std::cout << "time Out, Bye!" << std::endl;
                        sleep(2);
                        window.close();
                    }
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

/// --- HIJO 2 ---
void UnLoadWhatClient(struct Data* datos,int semID, sembuf *sBuf)
{
    int ctrl = -1;
    for(int i = 0; i < NUM_MESAS; ++i)
    {
        if(datos->GetTabureteState(i,semID,sBuf) == EMPEZANDO)
        {   ctrl = i;

            datos->SetTaburetes(i, COMIENDO, semID, sBuf);
            if(fork() == 0)
            {
                sleep(TIME_TODO);
                datos->SetTaburetes(ctrl, ACABADO, semID, sBuf);
                exit(0);
            }
        }
    }
}
/// --- PADRE ---
void ClockAlarm(int param)
{
    graficos.tiempoRestante--;
    alarm(1);
}

/// --- HIJO 1 ---

std::vector<sf::Color> LoadXML()
{
        std::vector<sf::Color> toReturn;
        rapidxml::xml_document<> xmlFile;
        std::ifstream file("config.xml");
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        std::string content(buffer.str());
        xmlFile.parse<0>(&content[0]);

        rapidxml::xml_node<> *pRoot = xmlFile.first_node();

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

            toReturn.push_back(newPedido);
        }

        return toReturn;
}

sf::Color GetNewClientColor(std::vector<sf::Color> &pedidos)
{
    sf::Color toReturn = sf::Color().Black;

    if (!pedidos.empty())
    {
        if(RANDOM_PEDIDOS == 0)
        {
            toReturn = pedidos.front();
            pedidos.erase(pedidos.begin());
        }
        else
        {
            int rnd = rand() % pedidos.size();
            toReturn = pedidos[rnd];
            pedidos.erase(pedidos.begin() +rnd);
        }
    }

    return toReturn;
}

int DrawNewClient(sf::Color color , Data *shData, int semId, struct sembuf *tempSemBuf)
{
    if (RANDOM_TABURETES == 0)
    {
        int i = 0;

        while(i  < 4 && shData->GetTabureteState(i,semId,tempSemBuf) != ESPERA)
        {
            i++;
        }

        if (i < 3)
        {
            graficos.OcupaTaburete(i);
            graficos.aPedidosADibujar[i].setFillColor(color);

            return i;
        }
    }
    else
    {
        std::vector<int>  emptyChair = std::vector<int>();

        for(int i = 0; i < NUM_MESAS; i++)
        {
            if (shData->GetTabureteState(i,semId,tempSemBuf) == ESPERA) emptyChair.push_back(i);
        }

        int nuevoSitio = rand() % emptyChair.size();
        graficos.OcupaTaburete(emptyChair[nuevoSitio]);
        graficos.aPedidosADibujar[emptyChair[nuevoSitio]].setFillColor(color);
        return emptyChair[nuevoSitio];
    }
}
