#include <iostream>

//temario
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <wait.h>
//
#include <vector>
#include <fstream>
#include<sstream>
#include <unistd.h>
#include "GraficoSFML.h"
#include <SFML/Graphics.hpp>


#include "../XML/rapidxml.hpp"

//constantes
#define WINDOW_H 800
#define WINDOW_V 600
#define TITLE "Mi super practica 1.1"

int fdS0[2]; // 0 lectura, 1escritura
int fdS1[2];
pid_t son1;
pid_t son0;

//pedidos de los clientes
std::vector<sf::Color> pedidos;
GraficoSFML graficos;

// --- FUNCIONES
void ClockAlarm(int param);

//se triggerea cuando tiene un sitio libre
void TriggerAlarm(int param);

//se ejecuta cuando el hijo 0 recibe un SIGALRM
void CargarCliente(int param);
//se ejecuta cuando el hijo 2 recibe un SIGUSR2
void UnLoadClient(int param);
//Se ejecuta cuando el hijo
int firstStoolFree();

//codigo string de los clientes "color,sitio"

int main()
{
    int statusPipeS0 = pipe(fdS0);
    if (statusPipeS0 <0) throw "error en pipe 1";


    sf::RenderWindow window(sf::VideoMode(WINDOW_H,WINDOW_V), TITLE);

    //llegada de clientes
    if((son0 = fork()) == 0){

        signal(SIGUSR1, TriggerAlarm);
        signal(SIGALRM, CargarCliente);

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

            pedidos.push_back(newPedido);
        }
        //escribe que cliente hay que dibujar
        while(1){
        //mirar el fdS0 a ver si hay sitio libre
        //if yes

        //else
            pause();
        }
        exit(0);

    }else {

        int statusPipeS1 = pipe(fdS1);
        if (statusPipeS1 <0) throw "error en pipe 2";

    //control de vaciado de clientes
        if((son1 = fork()) == 0){

            while(1){
            pause();
            }
            exit(0);
        }
        else{
            signal(SIGALRM,ClockAlarm);
            //signal(SIGUSR1, CargarCliente);
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
    alarm(5);
}

void CargarCliente(int param){
//SIG1
    sf::Color auxCol = sf::Color::White;
    std::string aux;
    char * buffer;
    int colInt[3];

    //Leemos los 9 numeros que componen el color de la comida
    read(fdS0[0],buffer,9);

     //buffer = "255000000";

      for(size_t x {0}; x < 3; x++)
    {
        aux = {buffer[x*3], buffer[x*3+1], buffer[x*3+2]};
        colInt[x] = std::stoi(aux);
    }

    auxCol = sf::Color(colInt[0], colInt[1], colInt[2], 255);
    DrawClient(auxCol);
}

void UnLoadClient(int param){
//alarm(5)
//mandar SIGUSR2 al padre

}
