#include <iostream>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <string>
#include <vector>


enum class States{MENU,INGAME,EXIT,MAX};


struct PlayerInfo
{
    PlayerInfo(std::string _n, std::string _id, int _g) : name(_n),id(_id),gold(_g){};

    std::string name;
    std::string id;
    int gold;


};

PlayerInfo Login(sql::Connection* conn, sql::Statement* stmt)
{
       std::string userName;
       std::string pass;
       sql::ResultSet* user;


       do
       {

           std::system("clear");
           std::cout << "Escribe el nombre de usuario o nuevo" << std::endl;
           std::cin >> userName ;

           int exist;
           if (userName == "Nuevo" || userName == "nuevo" || userName == "NUEVO")
           {
               bool ctrl = true;
               do
               {
                    std::system("clear");
                    std::cout << "Nuevo Usuario:\n";
                    std::cout << "Escibe el nombre del nuevo usuario o exit para salir\n";
                    std::cin >> userName;
                    sql::ResultSet* result = stmt->executeQuery("SELECT count(*)FROM Players_Data WHERE  PlayerUser = '"+userName+"'");
                    result->next();
                    exist = result->getInt(1);
                    if (userName == "exit"|| userName == "Exit" || userName == "EXIT")
                    {
                        ctrl = false;
                    }
                    else if (exist == 0)
                    {
                        std::cout << "Escribe la constraseña\n";
                        std::cin >> pass;
                        std::cout << "Confirma la contraseña\n";
                        std::string confirmPass;
                        std::cin >> confirmPass;

                        if (pass == confirmPass)
                        {
                            int numRows = stmt->executeUpdate("Insert into Players_Data(PlayerUser,PlayerPass) values ('" + userName +"','"+ pass+"')");
                            if (numRows == 1)
                            {
                                std::cout << "Nou Usuari: " << userName << ", amb password :" << pass << "\n";
                                sleep(2);
                            }

                            ctrl = false;
                        }
                        else
                        {
                            std::cout << "no coinciden las contrassenyas\n";
                        }
                    }
                    else
                    {
                        std::cout << "El usuario ya existe, proba con otro nombre" << "\n";
                        sleep(2);

                    }

               }while(ctrl);
            }

           else
           {
                    sql::ResultSet* result = stmt->executeQuery("SELECT count(*)FROM Players_Data WHERE  PlayerUser = '"+userName+"'");
                    result->next();
                    exist = result->getInt(1);

                    if (exist == 0)
                    {
                        std::cout << "No existe el usuario ( " + userName + " ) \n";
                        sleep(1);
                    }
                    else
                    {
                        std::cout << "Escribe la constraseña para el usuario " + userName + "\n";
                        std::cin >> pass;
                        sql::ResultSet* result = stmt->executeQuery("SELECT count(*)FROM Players_Data WHERE  PlayerUser = '"+userName+"' and PlayerPass = '"+pass+"'");
                        result->next();
                        exist = result->getInt(1);

                        if (exist == 0)
                        {
                            std::cout << "Login Failed \n";
                            sleep(2);
                        }
                        else
                        {
                            std::system("clear");
                            std::cout << "Correct LOGIN ! \n\n";
                            sleep(2);
                        }

                    }

           }

           user = stmt->executeQuery("SELECT count(*)FROM Players_Data WHERE  PlayerUser = '"+userName+"' and PlayerPass = '"+pass+"'");
           user->next();

       }while(!user->getInt(1));

        user = stmt->executeQuery("SELECT * FROM Players_Data WHERE PlayerUser = '"+userName+"' and PlayerPass = '"+pass+"'");
        user->next();
        PlayerInfo player = PlayerInfo(user->getString("PlayerUser"),user->getString("PlayerID"),user->getInt("Gold"));
        return player;
}


int main()
{
try{

       sql::Driver* driver = sql::mysql::get_driver_instance();
       //sql::Connection* conn = driver->connect("10.38.0.23","root","linux123");
       sql::Connection* conn = driver->connect("localhost","root","linux123");

       conn->setSchema("MazeGo");//"MazeGo");
       sql::Statement* stmt = conn->createStatement();

        PlayerInfo player =  Login(conn,stmt);

        ///---*---PlayerInfo---*---///

        std::cout << "Usuario : " << player.name << "\n";
        std::cout << "Oro : " << player.gold << "\n\n";

        std::cout << "*--POKEMONS--*\n";

        //sql::ResultSet* res = stmt->executeQuery("SELECT * FROM Pokemon_Data , Pokemon_List Where 'Pokemon_List.PlayerID_FK' = '"+ user->getString("PlayerID") +"' and  'Pokemon_List.PokemonID_FK' = 'Pokemon_Data.PokemonID' ");
        sql::ResultSet* res = stmt->executeQuery("SELECT * FROM Pokemon_Data, Pokemon_List WHERE Pokemon_List.PlayerID_FK = '"+  player.id +"' AND Pokemon_Data.PokemonID = Pokemon_List.PokemonID_FK ");

        if (res->next())
        {
            do{
                std::cout << " * Nombre = " << res->getString("Name") << "  |  ";
                std::cout << "Tipo = " << res->getString("Type") << "  |  ";
                std::cout << "Cantidad = " << res->getString("Quantity") << "\n";

            }while(res->next());
        }
        else
        {
            std::cout << " Este Usuario no dispone de ningun Pokemon\n";
        }

        res= nullptr;
        delete(res);

        sleep(5);

        States gameState = States::MENU;

        bool ctrl = true;
        ///---*---GameLoop---*---///
        do
        {
            switch(gameState)
            {
                case States::MENU:
                {
                    std::system("clear");
                    std::cout << "*^* MazeGo *^* \n\n";

                    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM Maps_Data");
                    int i= 0;
                    std::vector<std::string> maps;

                    while(res->next())
                    {
                        i++;
                        maps.push_back(res->getString("MapName"));
                        std::cout << "* "<< i << " : " << maps[i-1] << "\n";
                    }
                    i++;
                    maps.push_back("Exit");
                    std::cout << "* "<< i << " : " << maps[i-1] << "\n";


                   // std::cin >> ;
                    sleep(4);
                    gameState == States::EXIT;

                }
                break;

                case States::INGAME:
                {

                }
                break;
                case States::EXIT:
                {
                    ctrl = false;

                     std::system("clear");
                     std::cout << "BYEE ! \n";
                     sleep(1);
                }
                break;


            }

        }while(ctrl);



       /*result->close();
       delete(result);*/
       stmt->close();
       delete(stmt);
       conn->close();
       delete(conn);

}
catch(sql::SQLException& e)
{
    std::cout << "salto excepcion" << std::endl;
}
   return 0;
}
