#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "../XML/rapidxml.hpp"

#include "GraficoSFML.h"


int main()
{
    sf::TcpSocket socket;

    sf::Socket::Status status = socket.connect("10.38.0.23", 50000, sf::seconds(2.f));

    if (status != sf::Socket::Done)
    {
        // No s'ha pogut connectar
        std::cout << "Fail \n";
    }
    else
    {
        std::cout << "Good job \n";
    }
    return 0;
}
