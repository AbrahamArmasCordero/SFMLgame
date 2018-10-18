#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <time.h>

//temario
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <wait.h>

#include "GraficoSFML.h"
#include <SFML/Graphics.hpp>
#include "../XML/rapidxml.hpp"

//constantes
#define WINDOW_H 800
#define WINDOW_V 600
#define TITLE "Mi super practica 1.1"

#define MAX_BUFFER_RGB 9
#define MAX_BUFFER_S2 3

#define TIME_TODO 5

/// AUTORES: DAVID AULADELL, MAURIZIO CARLOTTA, ABRAHAM ARMAS CORDERO

int fdS1[2]; // 0 lectura, 1escritura
pid_t son1;
int fdS2[2];
pid_t son2;

/// --- HIJO 1 ---
std::queue<sf::Color> pedidos; //pedidos de los clientes

void TriggerAlarm(int param);
void CargarClienteSon(int param);//se ejecuta cuando el hijo 1 recibe un SIGALRM
void CargarClienteFath(int param);
bool waitingForClient = true;

/// --- HIJO 2 ---
std::queue<int> TabureteComiendo = std::queue<int>();//se ejecuta cuando el hijo 2 recibe un SIGUSR2

void UnLoadWhatClient(int param);
void UnLoadClient(int param);

/// --- PADRE ---
GraficoSFML graficos;

void ClockAlarm(int param); //se triggerea cuando el padre le panda al hijo 0 un SIGUSR1
int FirstStoolFree();
void DrawClient(sf::Color color);
void VaciarMesa(int param);

int main()
{
    srand(time(NULL));
    int statusPipeS1 = pipe(fdS1);
    if (statusPipeS1 < 0) throw "error en pipe 1";

    /// --- LLEGADA DE CLIENTES ---
    if((son1 = fork()) == 0){

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

            pedidos.push(newPedido);
        }

        while(1){ pause(); }

        exit(0);

    }else {
        int statusPipeS2 = pipe(fdS2);
        if (statusPipeS2 < 0 ) throw "error en pipe 2";

    /// --- CLIENTES COMIENDO ---
        if((son2 = fork()) == 0){
            //recibo señal SIGUSR2
            //leo mi fdP2
            //almaceno el asiento ocupado
            //Hago un hijo que espera 5 segundos y em envia un alarm 5 segundos
            //vaciar un asiento
            //vuelta al loop de pausa
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
            //exit
            return 0;
       }
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

        std::string rgb = std::to_string(pedidos.front().r) +  std::to_string(pedidos.front().g) + std::to_string(pedidos.front().b);
        n = rgb.c_str();
        write(fdS1[1],n,MAX_BUFFER_RGB);

            pedidos.pop();


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
void DrawClient(sf::Color color)
{
     //Llenamos el taburete en nuestra array

     /*int i = 0;

     while(i  < 4 && graficos.TabureteOcupado(i))
     {
      i++;
     }

     if (i < 3)
     {
        graficos.OcupaTaburete(i);
        graficos.aPedidosADibujar[i].setFillColor(color);
        waitingForClient = true;
     }*/

     if(!graficos.RestauranteLLeno())
    {
        //Llenamos el taburete en nuestra array

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

int FirstStoolFree()
{
    for(size_t x = 0; x < 3; x++)
    {
        if(!graficos.stoolState[x])
        {
            return x;
        }
    }
    std::cout << "Error en FirstStoolFree()" << std::endl;
    return -1;
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

void ClockAlarm(int param)
{
    graficos.tiempoRestante--;
    alarm(1);
}

