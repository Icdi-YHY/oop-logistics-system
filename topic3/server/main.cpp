#include "../include/Server.h"
#include <windows.h>
#include <iostream>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const int PORT = 8888;

    std::cout << "========================================" << std::endl;
    std::cout << "  Logistics Management System - Server" << std::endl;
    std::cout << "========================================" << std::endl;

    Server server(PORT);
    server.start();

    std::cout << "服务器已停止。" << std::endl;
    system("pause");
    return 0;
}
