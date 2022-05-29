#include"Server/Webserver.h"

int main()
{
    Webserver MyServer(
        1314, 3, 1000, false,
        3306, "root", "root", "webserver",
        12, 6, true, 1, 1024
    );

    MyServer.Start();
    
    return 0;
}