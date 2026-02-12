#include <crow.h>

int main()
{
    crow::SimpleApp TheShade; 

    CROW_ROUTE(TheShade, "/")([](){
        return "Hello world";
    });

    TheShade.port(18080).multithreaded().run();
}