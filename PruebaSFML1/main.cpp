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
static int fdS0[2]; // 0 lectura, 1escritura
static int fdS1[2];
pid_t son1;
pid_t son0;

GraficoSFML graficos;
// --- FUNCIONES
void ClockAlarm(int param);
//se ejecuta cuando el hijo 0 recibe un SIGALRM (cada 5 segundos)
void CargarCliente(int param);
//se ejecuta cuando el hijo 2 recibe un SIGUSR2
void UnLoadClient(int param);

//codigo string de los clientes "color,sitio"

int main()
{
    int statusPipeS0 = pipe(fdS0);
    if (statusPipeS0 <0) throw "error en pipe 1";
    rapidxml::xml_document<> xmlFile;
    std::ifstream file("config.xml");
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::string content(buffer.str());
    xmlFile.parse<0>(&content[0]);

    rapidxml::xml_node<> *pRoot = xmlFile.first_node();

    std::vector<sf::Color> pedidos;
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

    sf::RenderWindow window(sf::VideoMode(WINDOW_H,WINDOW_V), TITLE);

    //llegada de clientes
    if(son0 = fork() == 0){
        //escribe que cliente hay que dibujar
        while(1){
        //mirar el fdS0 a ver si hay sitio libre
        //if yes
            signal(SIGALRM, CargarCliente);
        //else
            pause();
        }
        exit(0);

    }else {
    //control de vaciado de clientes
        int statusPipeS1 = pipe(fdS1);
        if (statusPipeS1 <0) throw "error en pipe 2";
        if(son1 = fork()== 0){

        }
        else{
            sf::RenderWindow window(sf::VideoMode(WINDOW_H,WINDOW_V), TITLE);
            signal(SIGALRM,ClockAlarm);
            alarm(1);

            while(window.isOpen())
            {
                sf::Event event;
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
                            graficos.MueveJugador(sf::Vector2f(event.mouseButton.x, event.mouseButton.y));

                                for(int i = 1; i < 4; i++)
                                {
                                    if (graficos.aObjetosADibujar[graficos.aObjetosADibujar.size() - i].getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y)))
                                    {
                                        graficos.CogeComida(graficos.aObjetosADibujar[graficos.aObjetosADibujar.size() - i].getFillColor());

                                    }
                                }
                            }
                    }
                }

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

void ClockAlarm(int param)
{
    graficos.tiempoRestante--;
    alarm(1);
}

void CargarCliente(int param){
//Acceso a los clientes predeterminados
//vaciar el fdS0 y escribir la info del nuevo cliente
//mandar SIGUSR1 al padre
}

void UnLoadClient(int param){
//alarm(5)
//mandar SIGUSR2 al padre

}
