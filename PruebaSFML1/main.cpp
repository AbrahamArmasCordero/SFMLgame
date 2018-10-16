#include <iostream>
#include <SFML/Graphics.hpp>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include "GraficoSFML.h"

#define WINDOW_H 800
#define WINDOW_V 600
#define TITLE "Mi super practica 1.1"

GraficoSFML graficos;

void ClockAlarm(int param);

static int fdP1[2]; // 0 lectura, 1escritura
static int fdP2[2];
pid_t son1;
pid_t son0;

int main()
{
    int statusPipeS1 = pipe(fdP1);
    if (statusPipeS1 <0) throw "error en pipe 1";

    int statusPipeS2 = pipe(fdP2);
    if (statusPipeS2 <0) throw "error en pipe 2";


    //llegada de clientes
    if(son1 = fork() == 0){
        //escribe que cliente hay que dibujar
    }else {
    //control de vaciado de clientes
        if(son2 = fork()== 0){

        }
        else{

            while(window.isOpen())
            {
                sf::RenderWindow window(sf::VideoMode(WINDOW_H,WINDOW_V), TITLE);
                signal(SIGALRM,ClockAlarm);
                alarm(1);
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
        }
    }
    return 0;
}

void ClockAlarm(int param)
{
    graficos.tiempoRestante--;
    alarm(1);
}
