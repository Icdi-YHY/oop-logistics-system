#include "../include/Client.h"
#include <windows.h>
#include <iostream>
#include <exception>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    try {
        std::cout << "========================================" << std::endl;
        std::cout << "       物流管理系统 - 客户端" << std::endl;
        std::cout << "========================================" << std::endl;

        Client client;

        std::string host;
        std::cout << "服务器IP (回车默认 127.0.0.1): ";
        std::getline(std::cin, host);
        if (host.empty()) {
            host = "127.0.0.1";
        }

        if (!client.connect(host, 8888)) {
            std::cerr << "连接服务器失败，请确认服务器已启动。" << std::endl;
            system("pause");
            return 1;
        }

        std::cout << "已连接到服务器 " << host << ":8888" << std::endl;

        client.run();
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "程序发生未知异常" << std::endl;
    }

    std::cout << "客户端已退出。" << std::endl;
    system("pause");
    return 0;
}
