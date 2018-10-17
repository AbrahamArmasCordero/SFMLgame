#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>

//temario
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <wait.h>
//

#include "GraficoSFML.h"
#include <SFML/Graphics.hpp>

#include "../XML/rapidxml.hpp"

//constantes
#define WINDOW_H 800
#define WINDOW_V 600
#define TITLE "Mi super practica 1.1"

#define MAX_BUFFER_SIZE 5

int fdS0[2]; // 0 lectura, 1escritura
int fdS1[2];
pid_t son1;
pid_t son0;


GraficoSFML graficos;
//pedidos de los clientes
std::queue<sf::Color> pedidos;

// --- FUNCIONES
void ClockAlarm(int param);

//se triggerea cuando el padre le panda al hijo 0 un SIGUSR1
void TriggerAlarm(int param);

//se ejecuta cuando el hijo 0 recibe un SIGALRM
void CargarClienteSon(int param);
void CargarClienteFath(int param);
//codigo string de los clientes "color,sitio"

//se ejecuta cuando el hijo 2 recibe un SIGUSR2
std::queue<int> TabureteComiendo = std::queue<int>();

void UnLoadWhatClient(int param);
void UnLoadClient(int param);
//Se ejecuta cuando el hijo
int firstStoolFree();

int main()
{
    int statusPipeS0 = pipe(fdS0);
    if (statusPipeS0 <0) throw "error en pipe 1";

    /// --- LLEGADA DE CLIENTES ---
    /// --- LLEGADA DE CLIENTES ---
    if((son0 = fork()) == 0){

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

        int statusPipeS1 = pipe(fdS1);
        if (statusPipeS1 <0) throw "error en pipe 2";

    /// --- CLIENTES COMIENDO ---
        if((son1 = fork()) == 0){
        //recibo señal SIGUSR2 DONE
        //leo mi fdP2 DONE
        //almaceno el asiento ocupado DONE
        //espero 5 segundos
        //vaciar un asiento
        //vuelta al loop o pausa
            signal(SIGALRM, UnLoadClient);
            signal(SIGUSR2, UnLoadWhatClient);

            while(1){ pause(); }

            exit(0);
        }
        else{
            sf::RenderWindow window(sf::VideoMode(WINDOW_H,WINDOW_V), TITLE);

            signal(SIGALRM,ClockAlarm);
            signal(SIGUSR1, CargarClienteFath);
            ///signal(SIGUSR2, /*funcion hijo 1*/)
            alarm(1);

             //VARIABLES

            sf::Event event;

            bool mouseLeftButtPressed = false;
            bool mouseRightButtPressed = false;
            bool waitingForClient = false;

            std::string buffer;

            while(window.isOpen())
            {



                //EVENTOS

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

                //UPDATE

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
                                        graficos.aTaburetesADibujar[i].setFillColor(TABURETE_OCUPADO);
                                        write(fdS1[1],buffer.c_str(),1);
                                        buffer = "ERROR";
                                        //kill(son1,SIGUSR2); DESCOMENTAR-**************************************

                                        graficos.DejaComida(graficos.aPedidosADibujar[i].getFillColor());

                                    }

                                }
                            }
                            mouseLeftButtPressed = false;
                    }
                    else if(mouseRightButtPressed)
                    {
                        graficos.TiraComida();
                        graficos.MueveJugador(sf::Vector2f(event.mouseButton.x, event.mouseButton.y));
                    }


                    //Comprobación mesa libre
                    if(!waitingForClient && firstStoolFree() != -1)
                    {
                        kill(son0, SIGUSR1);
                        waitingForClient = true;
                    }
                }


                //DRAW

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

                graficos.UpdateTimer(graficos.aTextosADibujar);
                window.draw(graficos.jugador);
                window.draw(graficos.manoIzquierda);
                window.draw(graficos.manoDerecha);

                window.display();

                }

                if (graficos.tiempoRestante <= 0)
                window.close();
            }
            //esperar a todos los hijos
            //matarlos
            kill(son0, SIGKILL);
            kill(son1, SIGKILL);
            //exit
       }
    }
    return 0;
}

int firstStoolFree()
{
    for(size_t x = 0; x < 3; x++)
    {
        if(!graficos.stoolState[x])
        {
            return x;
        }
    }
    std::cout << "Error en firstStoolFree()" << std::endl;
    return -1;
}

void DrawClient(sf::Color color)
{
     //Llenamos el taburete en nuestra array
    int taburete = graficos.stoolState[firstStoolFree()];
    graficos.stoolState[taburete] = taburete;
    graficos.aTaburetesADibujar[taburete].setFillColor(TABURETE_OCUPADO);

    graficos.aPedidosADibujar[taburete].setFillColor(color);
}

void ClockAlarm(int param)
{
    graficos.tiempoRestante--;
    alarm(1);
}

void TriggerAlarm(int param)
{
    alarm(1);
}

void CargarClienteSon(int param)
{
    char* n;
    if (pedidos.size())
    {
        std::string rgb = std::to_string(pedidos.front().r) +  std::to_string(pedidos.front().g) + std::to_string(pedidos.front().b);
        n = rgb.c_str();
        write(fdS0[1],n,9);

        pedidos.pop();
    }
    else
    {
        n = '0';
        write(fdS0[1],n,1);
    }

    kill(getppid(), SIGUSR1);
    std::cout << "HEYEYEYYEYEYY" << std::endl;
}

void CargarClienteFath(int param){
//SIG1

    sf::Color auxCol = sf::Color::White;
    std::string aux;
    char * buffer;
    int colInt[3];

    //Leemos los 9 numeros que componen el color de la comida
    size_t is = read(fdS0[0],buffer, 9);
    buffer[is] = '\0';
    std::cout << is << std::endl;
    std::cout << buffer << std::endl;

     //buffer = "255000000";

     int control = 0;

  for(size_t x {0}; x < 3; x++)
    {

        if(buffer[control] == '0')
        {
            aux = {"0"};
            control++;
            std::cout << "guay" << std::endl;

        }
        /*else
        {
            aux = {buffer[control], buffer[control+1], buffer[control+2]};
            control += 3;
        }*/
          //colInt[x] = std::stoi(aux);
          //std::cout << colInt[x] << std::endl;

    }

    //auxCol = sf::Color(colInt[0], colInt[1], colInt[2], 255);
    //DrawClient(auxCol);
}

void UnLoadWhatClient(int param)
{
    char buffer[MAX_BUFFER_SIZE];
    size_t t = read(fdS1[0],buffer, MAX_BUFFER_SIZE);
    buffer[t] = '\0';
    int n = std::stoi(buffer);
    TabureteComiendo.push(n);
    alarm(5);
}
void UnLoadClient (int param)
{
    char* n = std::to_string(TabureteComiendo.front()).c_str();

    write(fdS1[1],n,1);

    TabureteComiendo.pop();
    kill(getppid(), SIGUSR2);
}
