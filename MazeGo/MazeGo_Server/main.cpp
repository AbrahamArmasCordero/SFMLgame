#include <iostream>
#include <SFML/Network.hpp>

int main()
{
    sf::TcpListener dispatcher;
    sf::Socket::Status status = dispatcher.listen(50000);
    sf::TcpSocket incoming;

    char data[100];
    std::size_t received;

    while(true)
    {
        if(status != sf::Socket::Done)
        {
            std::cout << "No se puede vincular al puerto 5000 \n";
        }


        if(dispatcher.accept(incoming) != sf::Socket::Done)
        {
            std::cout << "Error en aceptar conexión \n";
        }


    }

    incoming.disconnect();
    dispatcher.close();
}
