#include <iostream>
#include <streambuf>
#include <string>
#include <vector>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>


enum class States{MENU,INGAME,EXIT,MAX};


struct PlayerInfo
{
    PlayerInfo(std::string _n, std::string _id, int _g) : name(_n),id(_id),gold(_g){};

    std::string name;
    std::string id;
    int gold;


};


struct Map
{
    Map(std::string _id, std::string _n, std::string _d) : id(_id),name(_n), description(_d){};
    std::string name;
    std::string id;
    std::string description;

    std::string sessionID;
};

PlayerInfo Login(sql::Connection* conn, sql::Statement* stmt)
{
       std::string userName;
       std::string pass;
       sql::ResultSet* user;


       do
       {

           std::system("clear");
           std::cout << "Bienvenido A Super Mega Ultra MazeGO !\nLog In: \n";
           std::cout << "Escribe el nombre de usuario o nuevo para crear un nuevo Usuario" << std::endl;
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

        std::cout << "*--POKEMONS--*\n\n";

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

        std::cout << "\nIntro para Continuar .\n";
        std::cin.ignore().get();

        States gameState = States::MENU;

        bool ctrl = true;
        Map currentMap("","","");

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
                        std::cout << "* "<< i << " : " << maps[i-1] << "\n" << res->getString("MapInfo")<<"\n\n";
                    }

                    i++;
                    maps.push_back("Exit");

                    std::cout << "* "<< i << " : " << maps[i-1] << "\n\n";

                    std::string input;

                    std::cout << "Escribe el indice del nivel para seleccionarlo\n";
                    std::cin >> input;

                    if (input.size() == 1)
                    {
                        for (int i = 0; i < maps.size(); i++)
                        {

                            if ((input == std::to_string(i + 1)))
                            {
                                input = maps[i];
                                if (input == "Exit") gameState = States::EXIT;
                                else
                                {
                                    res = stmt->executeQuery("SELECT * FROM Maps_Data WHERE MapName = '"+ input +"'");
                                    res->next();
                                    currentMap = Map(res->getString("MapID"), input,res->getString("MapInfo"));

                                    res = nullptr;
                                    delete(res);

                                    int checkInsert = stmt->executeUpdate("INSERT INTO Sessions (PlayerID_FK, MapaID_FK, ConectionDate, DesconectionDate) VALUES  ('"+player.id+"', '"+currentMap.id+"', CURRENT_TIMESTAMP, NULL)");
                                    if (checkInsert == 1)
                                    {
                                    std::cout << "godJob\n";
                                    }
                                    gameState = States::INGAME;

                                }
                            }
                        }
                    }
                }

                break;

                case States::INGAME:
                {
                    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM  Sessions WHERE PlayerID_FK = '"+ player.id +"' AND MapaID_FK = '"+ currentMap.id +"'");

                    while(res->next())
                    {
                        if (res->getString("DesconectionDate") == "")
                            currentMap.sessionID =res->getString("SesionID");

                    };

                    std::string input;

                    do
                    {
                        std::system("clear");
                        std::cout << "Current Map = " << currentMap.name <<  "\n\n" << currentMap.description << "\n";
                        std::cout << "\nescribe salir para volver al menu!\n";

                        std::cin >> input;
                    }
                    while(input != "salir");

                    stmt->executeUpdate("UPDATE Sessions SET DesconectionDate = CURRENT_TIMESTAMP WHERE SesionID = '"+currentMap.sessionID+"'");
                    gameState = States::MENU;
                }
                break;
                case States::EXIT:
                {

                     std::system("clear");
                     std::cout << "BYEE ! \n";
                     sleep(1);

                    ctrl = false;
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
    std::cout << "salto excepcion"  << e.getErrorCode() << std::endl;
}
   return 0;
}
