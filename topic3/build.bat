@echo off
chcp 65001 >nul
echo ============================================
echo  Building Logistics Management System
echo  Topic 3 - Network Version (CS Architecture)
echo ============================================
echo.

:: Ensure we're in the topic3 directory
cd /d "%~dp0"

:: Build Server
echo [1/2] Building Server...
g++ -std=c++11 -Wall -o server/server.exe ^
    server/main.cpp ^
    src/Server.cpp ^
    src/UserHandlers.cpp ^
    src/AdminHandlers.cpp ^
    src/CourierHandlers.cpp ^
    src/Person.cpp ^
    src/User.cpp ^
    src/Admin.cpp ^
    src/Courier.cpp ^
    src/Package.cpp ^
    src/NormalPackage.cpp ^
    src/FragilePackage.cpp ^
    src/BookPackage.cpp ^
    src/DataManager.cpp ^
    -I./include ^
    -lws2_32

if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Server build failed!
    pause
    exit /b 1
)
echo [OK] Server built successfully: server\server.exe

:: Build Client
echo [2/2] Building Client...
g++ -std=c++11 -Wall -o client/client.exe ^
    client/main.cpp ^
    src/Client.cpp ^
    -I./include ^
    -lws2_32

if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Client build failed!
    pause
    exit /b 1
)
echo [OK] Client built successfully: client\client.exe

echo.
echo ============================================
echo  Build Complete!
echo  Run server first:  server\server.exe
echo  Then run client:   client\client.exe
echo ============================================
pause
