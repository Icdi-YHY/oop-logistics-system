# 题目三：C/S 网络版物流系统 — 从零开始的实现思路

## 本文定位

假设你刚做完题目二（一个单机运行的物流管理系统），现在老师要求你把它变成"网络版"——服务端和客户端分离，多个客户端可以同时连接服务端操作。

**你可能会想：**
- 什么是 C/S 架构？
- 题目二的代码怎么改？
- 服务端和客户端怎么通信？
- 多个客户端同时操作怎么保证数据不乱？

本文从完全不会的视角，一步步解释**为什么要这么做、原理是什么、代码怎么写、哪些代码对应什么功能**。

---

## 一、我们要解决什么问题？

### 1.1 题目二的问题

题目二的程序长这样：

```
┌─────────────────────────┐
│    logistics.exe        │
│                         │
│  ┌───────────────────┐  │
│  │  Console UI        │  │  同一个人操作
│  │  (cin/cout 菜单)   │  │
│  └────────┬──────────┘  │
│           ▼              │
│  ┌───────────────────┐  │
│  │  Controllers       │  │  业务逻辑
│  │  (User/Admin/Courier)│  │
│  └────────┬──────────┘  │
│           ▼              │
│  ┌───────────────────┐  │
│  │  DataManager       │  │  数据读写
│  └────────┬──────────┘  │
│           ▼              │
│     users.txt            │  文件存储在本地
│     packages.txt         │
└─────────────────────────┘
```

**缺陷：**
- 所有操作都在一台电脑上，数据文件也在这台电脑上
- 只有一个人能操作（一个命令行窗口）
- 用户、管理员、快递员不能同时使用系统

### 1.2 题目三的目标

```
┌──────────┐    网络(TCP)    ┌──────────────────┐
│ 客户端A   │───────→────────│                  │
│(用户界面) │←───────────────│     服务端        │
└──────────┘                │                  │
┌──────────┐                │  ┌────────────┐  │
│ 客户端B   │───────→────────│  │DataManager │  │    users.txt
│(管理员)   │←───────────────│  │(唯一数据源) │──→ packages.txt
└──────────┘                │  └────────────┘  │
┌──────────┐                │                  │
│ 客户端C   │───────→────────│                  │
│(快递员)   │←───────────────│                  │
└──────────┘                └──────────────────┘
```

**核心变化：**
1. 程序拆成两个 exe：`server.exe`（服务端）和 `client.exe`（客户端）
2. 客户端只负责显示菜单和收集用户输入
3. 服务端负责所有业务逻辑和数据存储
4. 服务端同时服务多个客户端（多线程）

---

## 二、前置知识：TCP 通信原理

### 2.1 类比：打电话

TCP 通信就像打电话：

| 步骤 | 打电话 | TCP 通信 |
|------|--------|----------|
| 1 | 甲守在电话旁等来电 | 服务端创建 socket，监听端口（`listen`） |
| 2 | 乙拨打甲的号码 | 客户端连接服务端的 IP:端口（`connect`） |
| 3 | 甲接听，通话建立 | 服务端 `accept`，连接建立 |
| 4 | 双方说话/听话 | `send` 发数据，`recv` 收数据 |
| 5 | 挂断 | `closesocket` 关闭连接 |

### 2.2 服务端启动需要 5 个步骤

这是 Windows 下使用 Winsock2 的固定套路，每一步都不能少：

```
步骤1：WSAStartup()      ← 初始化 Windows 网络库（必须最先调用）
步骤2：socket()          ← 创建一个套接字（可以理解为"电话机"）
步骤3：bind()            ← 把套接字绑定到一个端口号（"告诉别人打这个号码"）
步骤4：listen()          ← 开始监听（"守在电话旁"）
步骤5：accept()          ← 接受客户端连接（"接起电话"），每来一个客户端就返回一个新 socket
```

**对应的代码**（[Server.cpp:30-67](src/Server.cpp#L30-L67)）：

```cpp
void Server::start()
{
    // 步骤1：初始化 Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 步骤2：创建 socket
    // AF_INET = 使用 IPv4
    // SOCK_STREAM = 使用 TCP（可靠传输）
    listenSocket_ = socket(AF_INET, SOCK_STREAM, 0);

    // 【技巧】允许端口重用，防止"Bind failed"
    BOOL reuse = TRUE;
    setsockopt(listenSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    // 步骤3：绑定端口
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;        // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 监听所有网络接口
    serverAddr.sin_port = htons(port_);      // 端口号，htons 转换字节序
    bind(listenSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // 步骤4：开始监听
    // SOMAXCONN = 系统允许的最大排队数
    listen(listenSocket_, SOMAXCONN);

    // 步骤5：循环接受连接
    while (running_) {
        SOCKET clientSocket = accept(listenSocket_, ...);  // 阻塞等待
        // 为这个客户端创建一个线程...
    }
}
```

### 2.3 客户端连接需要 3 个步骤

```cpp
bool Client::connect(const std::string& host, int port)
{
    // 步骤1：初始化（和服务端一样）
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 步骤2：创建 socket
    sock_ = socket(AF_INET, SOCK_STREAM, 0);

    // 步骤3：连接服务端
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(host.c_str());  // "127.0.0.1" → 数字
    ::connect(sock_, (sockaddr*)&serverAddr, sizeof(serverAddr));

    connected_ = true;  // 标记已连接
}
```

**关键点**：客户端只需要 `socket` + `connect`，不需要 `bind` 和 `listen`。因为是客户端主动去连接服务端，操作系统会自动分配一个临时端口。

### 2.4 发送和接收数据

连接建立后，双方通过 `send` 和 `recv` 通信：

```cpp
// 发送：把字符串变成字节流发出去
send(socket, "LOGIN|user|zhangsan|123\n", 26, 0);

// 接收：从字节流读回来
char buffer[8192];
int len = recv(socket, buffer, sizeof(buffer) - 1, 0);
// buffer 里就是服务端返回的 "OK|Login success\n"
```

**核心概念**：TCP 传输的是**字节流**，不是"消息"。你发两次 `send`，对方可能一次 `recv` 就全收到了，也可能要 `recv` 多次。所以需要一个**协议**来界定"一条消息到哪里结束"。

### 2.5 为什么需要 `SO_REUSEADDR`？

```
场景：你关闭了 server.exe，立刻重新启动
问题：操作系统说端口 8888 还在"占着"（TIME_WAIT 状态，默认等 2-4 分钟）
结果：bind() 失败 → "Bind failed" → 服务端启动失败

解决：socket 创建后调用
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, ...)
  告诉操作系统"这个端口可以立即重用"
```

---

## 三、核心设计：通信协议

### 3.1 为什么需要协议？

客户端和服务端之间的连接是**字节流**。客户端连上后，需要一个双方约定的"语言"来沟通：

- 客户端说："我要登录，用户名 zhangsan，密码 123"
- 服务端说："登录成功" 或 "密码错误"
- 客户端说："我要发快递，收件人 lisi，类型 1，重量 2.5"
- 服务端说："快递已发出，单号 42，费用 20 元"

这些"说"的内容必须格式化，才能用代码解析。

### 3.2 我们设计的协议

全部定义在 [Protocol.h](include/Protocol.h) 中，约 60 行代码。

**格式：**

```
请求格式：命令|参数1|参数2|...\n
成功响应：OK|消息\n数据\nEND\n
失败响应：ERR|消息\n
```

**分隔符、前缀、命令名全部定义为常量**，避免写错字：

```cpp
const char DELIMITER = '|';           // 分隔符
const std::string OK_PREFIX = "OK";   // 成功前缀
const std::string ERR_PREFIX = "ERR"; // 失败前缀
const std::string END_MARKER = "END"; // 数据结束标记

const std::string CMD_LOGIN = "LOGIN";
const std::string CMD_SEND_PACKAGE = "SEND_PACKAGE";
const std::string CMD_RECHARGE = "RECHARGE";
// ... 共约 30 个命令常量
```

**为什么用纯文本而不用 JSON 或二进制？**
- 纯文本人眼可读，调试时可以直接看日志
- 不需要引入第三方库
- 用 `|` 分隔、`\n` 结尾，C++ 的 `std::getline` 天然支持

### 3.3 协议使用示例

**用户登录：**
```
客户端发送：LOGIN|user|zhangsan|123456\n
服务端返回：OK|User login success, welcome 张三\n
```

**发快递：**
```
客户端发送：SEND_PACKAGE|lisi|1|2.5|衣物\n
服务端返回：OK|Package sent! ID: 3, Fee: 20 yuan, Balance: 80 yuan\n
```

**查看我的快递（需要返回多行数据）：**
```
客户端发送：QUERY_SENT\n
服务端返回：
OK|Your sent packages
1|zhangsan|lisi|2025-05-20|待揽收|易碎品|20元
2|zhangsan|wangwu|2025-05-21|已签收|普通|15元
END
```

客户端解析响应时，读到 `END` 就知道数据结束了。

### 3.4 协议辅助函数

```cpp
// 不用手动拼字符串，用辅助函数
buildRequest(CMD_LOGIN, {"user", "zhangsan", "123456"});
// 生成: "LOGIN|user|zhangsan|123456\n"

buildOkResponse("Login success");
// 生成: "OK|Login success\n"

buildErrResponse("Wrong password");
// 生成: "ERR|Wrong password\n"

buildDataResponse("Packages list", "1|...|20元\n2|...|15元\n");
// 生成: "OK|Packages list\n1|...|20元\n2|...|15元\nEND\n"
```

---

## 四、服务端：如何处理请求

### 4.1 整体架构

```
主线程                              工作线程1（客户端A）
  │                                    │
  ├─ accept() ──→ 拿到 clientSocket ──→ CreateThread ──→ handleClient(clientSocket, 1)
  │                                    │                  │
  ├─ accept() ──→ 拿到 clientSocket ──→ CreateThread ──→ handleClient(clientSocket, 2)
  │                                    │                  │
  ├─ accept() ──→ ...                  │                recv() → processRequest() → send()
  │                                                       │
  ...                                                   recv() → processRequest() → send()
                                                          │
                                                        recv() 返回 0（客户端断开）
                                                          │
                                                        closesocket() → 线程结束
```

**对应代码**（[Server.cpp:70-101](src/Server.cpp#L70-L101)）：

```cpp
while (running_) {
    // 阻塞等待客户端连接
    SOCKET clientSocket = accept(listenSocket_, ...);

    // 分配一个 ID（用于日志区分不同客户端）
    int cid = nextClientId_++;

    // 打包参数
    ThreadParam* param = new ThreadParam();
    param->server = this;
    param->clientSocket = clientSocket;
    param->clientId = cid;

    // 创建线程，每个客户端独立运行
    // lambda 是 C++11 的匿名函数，[] 表示不捕获外部变量
    HANDLE hThread = CreateThread(NULL, 0,
        [](LPVOID lpParam) -> DWORD {
            ThreadParam* p = (ThreadParam*)lpParam;
            p->server->handleClient(p->clientSocket, p->clientId);
            closesocket(p->clientSocket);  // 线程结束时关闭 socket
            delete p;                       // 释放参数
            return 0;
        },
        param, 0, NULL);

    CloseHandle(hThread);  // 关闭句柄 ≠ 结束线程
}
```

**关键点解释：**

`CreateThread` 创建线程后，新线程和主线程**同时运行**：
- 主线程：立即回到 `accept()` 等待下一个客户端
- 新线程：在 `handleClient()` 里处理当前客户端的请求

`CloseHandle(hThread)` 不是结束线程！它只是释放主线程对这个线程的"引用"。线程会自己运行到 lambda 结束。

`ThreadParam` 是一个小结构体，把多个参数打包成一个指针传给线程：

```cpp
struct ThreadParam {
    Server* server;         // 指向服务端对象，用于调用方法
    SOCKET clientSocket;    // 这个客户端的通信 socket
    int clientId;           // 客户端编号（用于日志）
};
```

### 4.2 每个客户端的工作循环

`handleClient` 是一个 `while(true)` 循环，不停地 `recv → 处理 → send → recv → ...`：

```cpp
void Server::handleClient(SOCKET clientSocket, int clientId)
{
    int userType = 0;             // 0=未登录 1=用户 2=管理员 3=快递员
    std::string sessionUsername;  // 登录后记住是谁

    char buffer[8192];
    while (running_) {
        memset(buffer, 0, sizeof(buffer));
        // 阻塞等待客户端发数据
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            // recv 返回 0  = 客户端正常关闭连接
            // recv 返回 -1 = 网络错误
            std::cout << "  [#" << clientId << " 断开] " << sessionUsername << std::endl;
            break;  // 退出循环，线程结束
        }

        // 把收到的字节流转成字符串
        std::string request(buffer);

        // 处理请求（核心！见下一节）
        std::string response = processRequest(request, userType, sessionUsername);

        // 把响应发回客户端
        send(clientSocket, response.c_str(), response.length(), 0);
    }
}
```

**三个关键状态变量：**
- `userType`：决定当前能执行什么命令（未登录只能 LOGIN/REGISTER，登录后按角色分权限）
- `sessionUsername`：记住当前登录的是谁，后续操作（如发快递）以此身份执行
- `running_`：服务端关闭时设为 false，所有线程退出

### 4.3 请求分发的核心逻辑

`processRequest` 是整个服务端的大脑——收到一条请求后，解析出命令名，按角色路由到对应的处理器。

**数据流：**

```
"LOGIN|user|zhangsan|123456\n"
        │
        ▼ processRequest()
        │
   用 '|' 拆分字符串：
   cmd = "LOGIN"
   params = {"user", "zhangsan", "123456"}
        │
        ▼ 根据 userType 分发
   userType == 0（未登录）
        │
        ▼ dispatchUnauthCommand()
   cmd == "LOGIN" → handleLogin("user", "zhangsan", "123456", ...)
        │
        ▼ handleLogin() 内部
   查找用户、验证密码、设置 userType 和 sessionUsername
        │
        ▼ 返回
   "OK|User login success, welcome 张三\n"
```

**对应代码**（[Server.cpp:185-229](src/Server.cpp#L185-L229)）：

```cpp
std::string Server::processRequest(const std::string& request,
                                    int& userType, std::string& sessionUsername)
{
    // --- 第1步：解析命令 ---
    std::istringstream ss(request);
    std::string cmd;
    std::getline(ss, cmd, '|');   // 读第一个 | 之前的部分作为命令名

    std::vector<std::string> params;
    std::string param;
    while (std::getline(ss, param, '|')) {  // 读剩余的 | 分隔的参数
        params.push_back(param);
    }

    // --- 第2步：加锁，保证线程安全 ---
    EnterCriticalSection(&cs_);

    std::string response;
    try {
        // --- 第3步：按角色分发 ---
        if (cmd == CMD_LOGOUT) {
            userType = 0;
            sessionUsername = "";
            response = buildOkResponse("Logged out");
        } else if (userType == 1) {
            response = dispatchUserCommand(cmd, params, sessionUsername);
        } else if (userType == 2) {
            response = dispatchAdminCommand(cmd, params);
        } else if (userType == 3) {
            response = dispatchCourierCommand(cmd, params, sessionUsername);
        } else {
            response = dispatchUnauthCommand(cmd, params, userType, sessionUsername);
        }
    } catch (...) {
        response = buildErrResponse("Server error");
    }

    LeaveCriticalSection(&cs_);

    // --- 第4步：如果是修改数据的操作，自动保存到文件 ---
    if (needsSave(cmd)) {
        EnterCriticalSection(&cs_);
        dataManager_.saveData();
        LeaveCriticalSection(&cs_);
    }

    return response;
}
```

**为什么这样设计？**

1. **按 `userType` 分发**：每种角色能执行的命令不同，分到不同函数里，代码清晰
2. **加锁**：多个客户端线程可能同时访问 DataManager，`CRITICAL_SECTION` 保证同一时刻只有一个线程在操作数据
3. **自动保存**：`needsSave()` 列出所有会修改数据的命令（注册、发快递、充值等），执行后自动调用 `saveData()` 写文件
4. **异常捕获**：handler 里的错误（如 `std::stoi` 转换失败）不会让服务端崩溃

### 4.4 四个分发函数

`dispatchUnauthCommand` 处理未登录状态（[Server.cpp:231-244](src/Server.cpp#L231-L244)）：

```cpp
// 未登录时只能：登录、注册、退出
std::string Server::dispatchUnauthCommand(...) {
    if (cmd == CMD_LOGIN)      return handleLogin(...);
    if (cmd == CMD_REGISTER)   return handleRegister(...);
    if (cmd == CMD_EXIT)       return buildOkResponse("Goodbye");
    return buildErrResponse("Please login first");  // 其他命令一律拒绝
}
```

`dispatchUserCommand` 处理用户操作（[Server.cpp:246-272](src/Server.cpp#L246-L272)）：

```cpp
// 登录用户能：发快递、收快递、查询、充值、改密码
std::string Server::dispatchUserCommand(...) {
    if (cmd == CMD_SEND_PACKAGE)     return handleSendPackage(...);
    if (cmd == CMD_RECEIVE_PACKAGE)   return handleReceivePackage(...);
    if (cmd == CMD_QUERY_SENT)       return handleQuerySent(username);
    if (cmd == CMD_GET_BALANCE)      return handleBalanceQuery(username);
    if (cmd == CMD_RECHARGE)         return handleRecharge(username, amount);
    // ... 大约 10 个命令
    return buildErrResponse("Invalid command");
}
```

Admin 和 Courier 的分发函数同理。

### 4.5 一个具体的 Handler：登录

以 `handleLogin` 为例（[UserHandlers.cpp:13-42](src/UserHandlers.cpp#L13-L42)），看一个 handler 函数的完整写法：

```cpp
std::string Server::handleLogin(const std::string& type,   // "user"/"admin"/"courier"
                                 const std::string& user,  // 用户名
                                 const std::string& pwd,   // 密码
                                 int& userType,            // 输出：设置登录状态
                                 std::string& sessionUsername) // 输出：设置会话身份
{
    // --- 用户登录 ---
    if (type == "user") {
        User* u = dataManager_.findUser(user);       // 从内存中找用户
        if (u == nullptr) return buildErrResponse("User not found");
        if (!u->CheckPassword(pwd)) return buildErrResponse("Wrong password");
        userType = 1;           // 标记为"已登录用户"
        sessionUsername = user; // 记住用户名，后续操作靠它识别身份
        return buildOkResponse("User login success, welcome " + u->GetName());
    }
    // --- 管理员登录 ---
    else if (type == "admin") {
        Admin& admin = dataManager_.getAdmin();
        if (user != admin.GetUsername()) return buildErrResponse("Admin not found");
        if (!admin.CheckPassword(pwd)) return buildErrResponse("Wrong password");
        userType = 2;
        sessionUsername = user;
        return buildOkResponse("Admin login success");
    }
    // --- 快递员登录 ---
    else if (type == "courier") {
        int courierId = std::stoi(user);  // 快递员用 ID 登录，需要转整数
        Courier* c = dataManager_.findCourier(courierId);
        if (c == nullptr) return buildErrResponse("Courier not found");
        if (!c->CheckPassword(pwd)) return buildErrResponse("Wrong password");
        userType = 3;
        sessionUsername = user;
        return buildOkResponse("Courier login success, welcome " + c->GetName());
    }
    return buildErrResponse("Invalid login type");
}
```

**注意 `userType` 和 `sessionUsername` 是引用传参**：handleLogin 修改它们后，`handleClient` 中的变量也变了。下一次这个客户端发请求时，`userType == 1`，`sessionUsername == "zhangsan"`，服务端就知道"这是 zhangsan 在操作"。

### 4.6 Handler：发送快递

以 `handleSendPackage` 为例（[UserHandlers.cpp:58-100](src/UserHandlers.cpp#L58-L100)），看业务逻辑怎么写：

```cpp
std::string Server::handleSendPackage(const std::string& sender, const std::string& receiver,
                                       int type, double weightOrCount, const std::string& desc)
{
    // 1. 检查收件人是否存在
    User* recvUser = dataManager_.findUser(receiver);
    if (recvUser == nullptr) return buildErrResponse("Receiver not found");

    // 2. 获取寄件人（就是当前登录用户）
    User* sendUser = dataManager_.findUser(sender);

    // 3. 根据类型创建不同的 Package 子类（多态！）
    Package* newPackage = nullptr;
    double price = 0;
    switch (type) {
    case 1:  // 易碎品：8元/kg
        price = 8.0 * weightOrCount;
        newPackage = new FragilePackage(id, sender, receiver, time, desc, weightOrCount);
        break;
    case 2:  // 图书：2元/本
        price = 2.0 * weightOrCount;
        newPackage = new BookPackage(id, sender, receiver, time, desc, (int)weightOrCount);
        break;
    case 3:  // 普通快递：5元/kg
        price = 5.0 * weightOrCount;
        newPackage = new NormalPackage(id, sender, receiver, time, desc, weightOrCount);
        break;
    }

    // 4. 检查余额
    if (sendUser->GetBalance() < price) {
        delete newPackage;
        return buildErrResponse("Insufficient balance");
    }

    // 5. 扣费 + 公司入账
    sendUser->DeductBalance(price);
    dataManager_.getAdmin().AddCompanyBalance(price);
    dataManager_.addPackage(newPackage);

    return buildOkResponse("Package sent! ID: " + id + ", Fee: " + ...);
}
```

**和题目二的区别：**
- 题目二：`sendPackage()` 里直接 `cout` 打印结果，用户看到后再 `cin` 输入
- 题目三：`handleSendPackage()` 返回字符串，服务端把它发回客户端，客户端显示

---

## 五、客户端：如何变成"哑终端"

### 5.1 客户端的定位

题目三的客户端**不包含任何业务逻辑**。它做的事情非常简单：

```
用户按键 → 拼命令字符串 → send() → recv() → 显示结果
```

类比：客户端就像一个**远程终端**，所有"聪明的事情"都在服务端做。

### 5.2 客户端的三个核心方法

```cpp
// 1. 发送命令
bool Client::sendRequest(const std::string& request) {
    return send(sock_, request.c_str(), request.length(), 0) != SOCKET_ERROR;
}

// 2. 接收响应
std::string Client::receiveResponse() {
    char buffer[8192];
    int bytesReceived = recv(sock_, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        return "ERR|连接断开";  // 服务端断开了
    }
    return std::string(buffer);
}

// 3. 显示响应
void Client::printResponse(const std::string& response) const {
    // 如果是 ERR|xxx → 显示 "错误: xxx"
    // 如果是 OK|xxx\n数据\nEND → 显示消息 + 数据行
}
```

### 5.3 客户端的一个完整操作

以"用户登录"为例（[Client.cpp:88-105](src/Client.cpp#L88-L105)）：

```cpp
void Client::handleUserLogin()
{
    // 第1步：收集用户输入
    std::string username, password;
    std::cout << "用户名: ";
    std::cin >> username;
    std::cout << "密码: ";
    std::cin >> password;

    // 第2步：拼命令字符串，发送
    // buildRequest 生成 "LOGIN|user|zhangsan|123456\n"
    sendRequest(buildRequest(CMD_LOGIN, {"user", username, password}));

    // 第3步：接收服务端响应
    std::string response = receiveResponse();

    // 第4步：根据响应决定下一步
    if (response.find("OK") == 0) {
        std::cout << "登录成功!" << std::endl;
        userMenu();  // 进入用户子菜单
    } else {
        std::cout << "登录失败: " << response << std::endl;
        // 返回主菜单
    }
}
```

**对比题目二的登录：**

```cpp
// 题目二：直接在函数里操作数据
void UserController::userLogin() {
    cin >> username >> password;
    User* u = dataManager_.findUser(username);  // ← 直接访问 DataManager
    if (u && u->CheckPassword(password)) {
        currentUser_ = u;                        // ← 直接修改状态
        showUserMenu();
    }
}

// 题目三：发命令给服务端，服务端做
void Client::handleUserLogin() {
    cin >> username >> password;
    sendRequest("LOGIN|user|" + username + "|" + password);  // ← 发命令
    string resp = receiveResponse();                          // ← 收结果
    if (resp starts with "OK") userMenu();
}
```

**本质变化**：题目二是"我自己查数据"，题目三是"我让服务端帮我查数据"。

### 5.4 两阶段操作：揽收

揽收需要两步交互（先看列表，再选择），这是"两阶段协议"的代表：

```cpp
void Client::courierCollect()
{
    // 阶段1：请求可揽收的列表
    sendRequest(buildRequest(CMD_COURIER_COLLECT, {}));  // 空参数 = "列出我待揽收的"
    std::string response = receiveResponse();
    printResponse(response);  // 显示列表

    // 用户看完列表后选择
    std::cout << "请输入要揽收的编号(逗号分隔, 0取消): ";
    std::string indices;
    std::cin >> indices;
    if (indices == "0" || indices.empty()) return;

    // 阶段2：发送揽收命令
    sendRequest(buildRequest(CMD_COURIER_COLLECT, {indices}));  // 带参数 = "揽收这些"
    printResponse(receiveResponse());
}
```

服务端的 `handleCourierCollect` 也对应两个阶段：
- 无参数 → 返回待揽收列表
- 有参数 → 执行揽收操作

---

## 六、多线程安全：锁

### 6.1 为什么需要锁？

服务端有多个客户端线程同时运行。想象这个场景：

```
线程A（用户zhangsan）：  dataManager_.findUser("lisi") → 扣费 → dataManager_.addPackage(...)
线程B（用户wangwu）：    dataManager_.findUser("lisi") → 扣费 → dataManager_.addPackage(...)
```

如果两个线程同时读到 `users_` 这个 `vector`，一个在修改、一个在读，程序会崩溃。

### 6.2 CRITICAL_SECTION 是什么？

Windows 提供的最简单的互斥锁。使用方式：

```cpp
// 初始化（构造函数中执行一次）
CRITICAL_SECTION cs_;
InitializeCriticalSection(&cs_);

// 加锁（只有一个线程能通过，其他线程在此等待）
EnterCriticalSection(&cs_);
// ... 操作共享数据 ...
LeaveCriticalSection(&cs_);

// 销毁（析构函数中执行）
DeleteCriticalSection(&cs_);
```

**类比**：`CRITICAL_SECTION` 就像厕所门锁——一个人进去后锁门，外面的人排队等。里面的人出来解锁，下一个才能进去。

### 6.3 在我们的代码中怎么用

```cpp
std::string Server::processRequest(...) {
    // ... 解析命令 ...

    EnterCriticalSection(&cs_);  // ← 锁！同一时刻只有一个线程能通过

    // 临界区：所有对 DataManager 的读写都在这之间
    // 包括 findUser、addPackage、deductBalance 等
    response = dispatchXXX(cmd, params, ...);

    LeaveCriticalSection(&cs_);  // ← 解锁

    // 文件保存单独加锁（因为写文件很慢，不想阻塞请求处理）
    if (needsSave(cmd)) {
        EnterCriticalSection(&cs_);
        dataManager_.saveData();
        LeaveCriticalSection(&cs_);
    }

    return response;
}
```

**为什么保存文件的锁和业务锁分开？**

写文件（`saveData`）比较慢（硬盘 I/O）。如果放在同一个锁里，一个客户端在写文件时，其他所有客户端都得等着。分成两个锁后，处理完业务就解锁，其他客户端可以继续处理，只有保存时才短暂加锁。

---

## 七、从题目二到题目三的代码映射

如果你已经写了题目二，下面告诉你哪些代码搬到哪里：

| 题目二的代码 | 题目三的位置 | 变化 |
|-------------|-------------|------|
| `main.cpp` 的菜单逻辑 | `Client.cpp` 的 `run()` + 各 handler | 不直接调 Controller，改为 `sendRequest` |
| `UserController` 的业务逻辑 | `Server.cpp` → `UserHandlers.cpp` | 去掉 `cout/cin`，改为返回字符串 |
| `AdminController` 的业务逻辑 | `Server.cpp` → `AdminHandlers.cpp` | 同上 |
| `CourierController` 的业务逻辑 | `Server.cpp` → `CourierHandlers.cpp` | 同上 |
| `DataManager` | `DataManager` | 不变，但加了锁保护 |
| `Person/User/Admin/Courier` | 不变 | 完全不变 |
| `Package/Fragile/Book/Normal` | 不变 | 完全不变 |

**核心经验**：题目三的"业务层"就是题目二去掉 UI 后的纯逻辑，再把 `cout`/`cin` 替换成字符串的收发。

---

## 八、代码文件对应功能速查

### 8.1 服务端文件

| 文件 | 负责什么 | 关键函数 |
|------|----------|----------|
| [server/main.cpp](server/main.cpp) | 入口 | 创建 Server，调用 start() |
| [src/Server.cpp](src/Server.cpp) | 网络 + 请求分发 | `start()`(监听+accept循环), `handleClient()`(recv/send循环), `processRequest()`(解析+路由+锁) |
| [src/UserHandlers.cpp](src/UserHandlers.cpp) | 用户 handler | `handleLogin`, `handleRegister`, `handleSendPackage`, `handleReceivePackage`, `handleQuerySent/Received/ById/ByTime`, `handleBalanceQuery`, `handleRecharge`, `handleChangePwd` |
| [src/AdminHandlers.cpp](src/AdminHandlers.cpp) | 管理员 handler | `handleAdminShowUsers/Packages/Balance/Couriers`, `handleAdminAddCourier`, `handleAdminRemoveCourier`, `handleAdminAssignCourier`, `handleAdminQueryBy*` |
| [src/CourierHandlers.cpp](src/CourierHandlers.cpp) | 快递员 handler | `handleCourierMyTasks`, `handleCourierCollect`, `handleCourierMyRecords`, `handleCourierBalance`, `handleCourierChangePwd` |
| [src/DataManager.cpp](src/DataManager.cpp) | 数据层 | `loadData/saveData`, `findUser`, `addPackage`, `findCourier`... |
| [include/Protocol.h](include/Protocol.h) | 协议常量 + 辅助函数 | `buildRequest()`, `buildOkResponse()`, `buildErrResponse()`, `CMD_*` 常量 |

### 8.2 客户端文件

| 文件 | 负责什么 | 关键函数 |
|------|----------|----------|
| [client/main.cpp](client/main.cpp) | 入口 | 连接 → run() → system("pause") |
| [src/Client.cpp](src/Client.cpp) | 连接 + 全部菜单 | `connect()`, `run()`, `userMenu()`, `adminMenu()`, `courierMenu()`, `handle*()` |
| [include/Client.h](include/Client.h) | 客户端类声明 | 所有菜单相关方法 |

### 8.3 共用文件（和题目二一致）

| 文件 | 内容 |
|------|------|
| `Person.h/cpp` | 人物基类（username, name, password, balance） |
| `User.h/cpp` | 用户（继承 Person） |
| `Admin.h/cpp` | 管理员（继承 Person） |
| `Courier.h/cpp` | 快递员（继承 Person） |
| `Package.h/cpp` | 抽象基类 |
| `FragilePackage.h/cpp` | 易碎品（8元/kg） |
| `BookPackage.h/cpp` | 图书（2元/本） |
| `NormalPackage.h/cpp` | 普通快递（5元/kg） |

---

## 九、常见问题

### Q1：服务端启动时为什么报 "Bind failed"？

**原因**：上一个 server.exe 的端口还没释放（操作系统 TIME_WAIT 机制）。

**解决**：代码已加 `SO_REUSEADDR`。如果还报错，打开任务管理器结束残留的 server.exe 进程。

### Q2：客户端为什么显示 "ERR|连接断开"？

**原因**：服务端没启动，或者服务端关闭了这个客户端的连接。

**检查步骤**：
1. 确认 server.exe 正在运行
2. 看服务端日志是否有 `[#N 断开]`
3. 如果服务端日志正常，检查客户端和服务端是否在同一台机器（都用 127.0.0.1）

### Q3：多个客户端同时发快递会不会出问题？

不会。`CRITICAL_SECTION` 保证同一时刻只有一个线程操作 DataManager。后面的客户端会排队等着。

### Q4：服务端能同时处理多少个客户端？

理论上取决于系统资源。实际测试中 10 个并发客户端正常工作。

### Q5：题目二的数据文件能直接在题目三用吗？

可以。题目二和题目三使用完全相同的数据文件格式，直接复制即可。

---

## 十、关键设计思路回顾

```
题目二                                    题目三
                                        
业务逻辑 + UI 混在一起        ──→    业务逻辑 → 服务端（返回字符串）
                                        UI → 客户端（收发字符串）
                                        
直接调用 DataManager          ──→    通过协议字符串通信
cout << result                         send("CMD|params")
cin >> input                           recv("OK|result")
                                        
单进程                         ──→    两个独立 exe，通过网络通信
                                        
无并发问题                     ──→    用 CRITICAL_SECTION 保护共享数据
                                        
函数直接操作对象               ──→    handler 函数接受字符串参数，返回字符串结果
```

**一句话总结**：题目三做的事情，就是把题目二的函数调用"拉远"到网络上——用字符串代替 C++ 函数调用，用 socket 代替直接内存访问，用锁保护多个线程同时读写数据。
