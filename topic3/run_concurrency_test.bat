@echo off
chcp 65001 >nul
cd /d "%~dp0"

echo ============================================
echo   物流系统并发测试 — 一键运行
echo ============================================
echo.

:: 1. 编译
echo [1/4] 编译服务端和客户端...
g++ -std=c++11 -Wall -o server\server.exe server\main.cpp src\Server.cpp src\UserHandlers.cpp src\AdminHandlers.cpp src\CourierHandlers.cpp src\Person.cpp src\User.cpp src\Admin.cpp src\Courier.cpp src\Package.cpp src\NormalPackage.cpp src\FragilePackage.cpp src\BookPackage.cpp src\DataManager.cpp -I./include -lws2_32 2>nul
g++ -std=c++11 -Wall -o client\client.exe client\main.cpp src\Client.cpp -I./include -lws2_32 2>nul
if not exist server\server.exe (
    echo [FAIL] 编译失败，请检查代码！
    pause
    exit /b 1
)
echo [OK] 编译成功
echo.

:: 2. 清理旧数据
echo [2/4] 清理旧数据...
del /q users.txt couriers.txt packages.txt admin.txt next_id.txt next_courier_id.txt 2>nul
echo [OK] 完成
echo.

:: 3. 启动服务端（后台）
echo [3/4] 启动服务端...
start "物流服务端" /MIN server\server.exe
timeout /t 2 /nobreak >nul
echo [OK] 服务端已启动
echo.

:: 4. 运行并发测试
echo [4/4] 运行并发测试...
echo ============================================
python concurrency_test.py
echo ============================================
echo.
echo 测试完成！按任意键关闭服务端并退出...
pause >nul

:: 5. 关闭服务端
taskkill /FI "WINDOWTITLE eq 物流服务端" /F 2>nul
del /q users.txt couriers.txt packages.txt admin.txt next_id.txt next_courier_id.txt 2>nul
