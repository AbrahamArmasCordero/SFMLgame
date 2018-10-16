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


int main()
{
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

    return 0;
}

void ClockAlarm(int param)
{
    graficos.tiempoRestante--;
    alarm(1);
}
