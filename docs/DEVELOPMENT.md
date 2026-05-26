# 物流管理系统 — 完整开发文档

## 项目概述

本项目是面向对象程序设计课程实验，通过三个递进式迭代（题目一/二/三），逐步展示 **封装 → 继承与多态 → C/S 网络架构与并发** 的 OOP 核心概念。

| 题目 | 核心概念 | 架构 | 新增内容 |
|------|----------|------|----------|
| 题目一 | 封装 | 单机单体 | User / Admin / Package 基础类 |
| 题目二 | 继承 + 多态 | 单机 MVC | Person 基类、Package 抽象基类+3 子类、Courier 快递员 |
| 题目三 | 网络 + 并发 | Client-Server (TCP) | Winsock 通信、多线程服务端、文本协议、Python 自动化测试 |

```
题目一（封装）──→ 题目二（继承+多态）──→ 题目三（C/S架构+并发）
  单体应用            MVC分层             网络分布式
  固定计费            多态计费              多客户端并发
  2种角色             3种角色                3种角色保持不变
  2种状态             3种状态                3种状态保持不变
```

---

## 一、题目一：基础物流系统（封装）

### 1.1 目录结构

```
topic1/
├── include/
│   ├── User.h              # 用户类
│   ├── Admin.h             # 管理员类
│   ├── Package.h           # 快递类
│   └── LogisticsSystem.h   # 系统主控类
├── src/
│   ├── main.cpp            # 程序入口
│   ├── User.cpp            # 用户实现
│   ├── Admin.cpp           # 管理员实现
│   ├── Package.cpp         # 快递实现
│   └── LogisticsSystem.cpp # 系统主控实现（约700行）
└── *.txt                   # 数据文件（运行时生成）
```

### 1.2 类设计

```
┌──────────────┐    ┌───────────────┐    ┌────────────────────────┐
│    User      │    │    Admin      │    │        Package         │
├──────────────┤    ├───────────────┤    ├────────────────────────┤
│- username_   │    │- username_    │    │- packageId_            │
│- name_       │    │- name_        │    │- sender_               │
│- phonenum_   │    │- password_    │    │- receiver_             │
│- password_   │    │- balance_     │    │- sendTime_             │
│- balance_    │    ├───────────────┤    │- receiveTime_          │
│- address_    │    │+ CheckPwd()   │    │- status_ (0待签收/1已签)│
├──────────────┤    │+ SetPwd()     │    │- description_          │
│+ GetBalance()│    │+ AddBalance() │    ├────────────────────────┤
│+ Recharge()  │    │+ ViewAllUsers│    │+ Sign()                │
│+ Deduct()    │    │+ ViewAllPkgs │    │+ IsWaitingSign()       │
│+ CheckPwd()  │    └───────────────┘    └────────────────────────┘
│+ SetPwd()    │
└──────────────┘
     注意：User 和 Admin 有大量重复代码（username_, name_, password_, balance_ 及其 getter/setter）
```

**设计特点：**
- `User` 和 `Admin` 各自独立实现，存在大量代码重复
- `Package` 为普通具体类，计费固定为 **15 元**（硬编码在 `LogisticsSystem::sendPackage()`）
- 快递状态仅 2 种：待签收(0) → 已签收(1)
- 所有业务逻辑集中在 `LogisticsSystem` 一个类中，约 700 行

### 1.3 数据文件

| 文件 | 格式 | 说明 |
|------|------|------|
| `users.txt` | `username\|name\|phone\|password\|balance\|address` | 用户数据 |
| `admin.txt` | `username\|name\|password\|balance` | 管理员数据 |
| `packages.txt` | `id\|sender\|receiver\|sendTime\|receiveTime\|status\|description` | 快递数据 |
| `next_id.txt` | `N` | 下一个快递单号 |

### 1.4 架构问题

1. **代码重复**：User 和 Admin 的 username_ / name_ / password_ / balance_ 重复定义
2. **缺少抽象**：Package 不支持多态，新增快递类型需要修改 LogisticsSystem
3. **职责不清**：LogisticsSystem 同时负责 UI、业务逻辑、数据持久化
4. **扩展困难**：新增角色（如快递员）需要大量修改

---

## 二、题目二：快递员任务管理系统（继承与多态）

### 2.1 目录结构

```
topic2/
├── include/
│   ├── Person.h            # 【新增】人物基类
│   ├── User.h              # 【修改】继承 Person
│   ├── Admin.h             # 【修改】继承 Person
│   ├── Courier.h           # 【新增】快递员，继承 Person
│   ├── Package.h           # 【修改】抽象基类
│   ├── NormalPackage.h     # 【新增】普通快递子类
│   ├── FragilePackage.h    # 【新增】易碎品子类
│   ├── BookPackage.h       # 【新增】图书子类
│   ├── DataManager.h       # 【新增】数据管理类
│   ├── UserController.h    # 【新增】用户控制器
│   ├── AdminController.h   # 【新增】管理员控制器
│   ├── CourierController.h # 【新增】快递员控制器
│   └── LogisticsSystem.h   # 【简化】主控外观类
└── src/ （对应 .cpp 文件）
```

### 2.2 类继承体系

```
                    ┌──────────────────┐
                    │     Person       │  ← 新增抽象基类
                    ├──────────────────┤
                    │- username_       │
                    │- name_           │
                    │- password_       │
                    │- balance_        │
                    ├──────────────────┤
                    │+ CheckPassword() │
                    │+ SetPassword()   │
                    │+ AddBalance()    │
                    │+ DeductBalance() │
                    └───────┬──────────┘
                            │
          ┌─────────────────┼─────────────────┐
          ▼                 ▼                  ▼
   ┌─────────────┐   ┌─────────────┐   ┌─────────────┐
   │    User     │   │    Admin    │   │   Courier   │
   ├─────────────┤   ├─────────────┤   ├─────────────┤
   │- phonenum_  │   │(无额外属性)  │   │- id_        │
   │- address_   │   ├─────────────┤   │- phone_     │
   ├─────────────┤   │+ ViewAll()  │   ├─────────────┤
   │+ Recharge() │   └─────────────┘   │+ GetId()    │
   └─────────────┘                     │+ GetPhone() │
                                       └─────────────┘

                    ┌─────────────────────┐
                    │   Package (abstract)│  ← 改为抽象基类
                    ├─────────────────────┤
                    │- packageId_         │
                    │- sender_            │
                    │- receiver_          │
                    │- sendTime_          │
                    │- receiveTime_       │
                    │- status_ (0/1/2)    │
                    │- description_       │
                    │- courierId_         │  ← 新增快递员关联
                    ├─────────────────────┤
                    │+ GetPrice() = 0     │  ← 纯虚函数
                    │+ Sign()             │
                    │+ IsWaitingCollect() │  ← 新增
                    │+ IsWaitingSign()    │
                    └─────────┬───────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        ▼                     ▼                      ▼
┌───────────────┐   ┌───────────────┐   ┌───────────────┐
│FragilePackage │   │  BookPackage  │   │NormalPackage  │
├───────────────┤   ├───────────────┤   ├───────────────┤
│- weight_      │   │- count_       │   │- weight_      │
├───────────────┤   ├───────────────┤   ├───────────────┤
│GetPrice():    │   │GetPrice():    │   │GetPrice():    │
│  8.0 × weight │   │  2.0 × count │   │  5.0 × weight │
└───────────────┘   └───────────────┘   └───────────────┘
```

### 2.3 MVC 分层架构

```
┌──────────────────────────────────────┐
│         LogisticsSystem (外观)        │
│  创建并持有 DataManager + 3 Controller │
│  Run() → 主菜单 → 委托给 Controller    │
└──────────────┬───────────────────────┘
               │
   ┌───────────┼───────────┐
   ▼           ▼           ▼
┌──────┐ ┌──────┐ ┌──────┐
│User  │ │Admin │ │Courier│  ← 控制器层（含菜单+业务逻辑）
│Ctrl  │ │Ctrl  │ │Ctrl   │
└──┬───┘ └──┬───┘ └──┬────┘
   │        │        │
   └────────┼────────┘
            ▼
    ┌──────────────┐
    │ DataManager  │  ← 数据层（文件读写+集合管理）
    ├──────────────┤
    │ users_       │
    │ couriers_    │
    │ packages_*   │  ← vector<Package*> 多态存储
    │ admin_       │
    │ nextId_      │
    └──────┬───────┘
           │
    ┌──────┴──────┐
    ▼      ▼      ▼
 users.txt  packages.txt  couriers.txt
 admin.txt  next_id.txt   next_courier_id.txt
```

**关键改进（相比题目一）：**
- `DataManager` 从 `LogisticsSystem` 中分离，单一职责
- Controller 层分离 UI 逻辑和业务逻辑
- `LogisticsSystem` 从 700 行缩减为外观类（仅创建对象 + 主菜单路由）

### 2.4 新增业务流程

#### 快递状态机（3 种状态）

```
待揽收(0) ──(快递员揽收)──→ 待签收(1) ──(用户签收)──→ 已签收(2)
```

**对比题目一**：题目一只有 2 种状态（待签收→已签收），没有揽收环节。

#### 发送快递（多态计费）

```
1. 用户登录
2. 选择类型：易碎品(8元/kg) / 图书(2元/本) / 普通(5元/kg)
3. 输入收件人、重量/数量、描述
4. 调用对应子类的 GetPrice() 计算费用
5. 扣费 → 状态=待揽收(0) → courierId=0
```

#### 快递员揽收（50% 分成）

```
【管理员】查看待揽收快递 → 选择快递 → 选择快递员 → 分配
【快递员】登录 → 查看我的任务 → 选择揽收 → 揽收费 = 快递费×50%
                                     ↓
         管理员公司余额 -= 揽收费      快递员账户 += 揽收费
```

### 2.5 数据文件变更

| 文件 | 变更 | 新格式 |
|------|------|--------|
| `packages.txt` | **修改** | `id\|sender\|receiver\|sendTime\|receiveTime\|status\|type\|detail\|description\|courierId` |
| `couriers.txt` | **新增** | `id\|name\|phone\|password\|balance` |
| `next_courier_id.txt` | **新增** | `N` |

**packages.txt 新字段说明**：type=1易碎品/2图书/3普通 | detail=重量或数量 | courierId=0未分配

---

## 三、题目三：物流管理系统 C/S 网络版

### 3.1 目录结构

```
topic3/
├── include/
│   ├── Person.h / User.h / Admin.h / Courier.h   # 同题目二
│   ├── Package.h / FragilePackage.h / BookPackage.h / NormalPackage.h  # 同题目二
│   ├── DataManager.h      # 【修改】模块化加载 + 线程安全接口
│   ├── Server.h           # 【新增】TCP 服务端
│   ├── Client.h           # 【新增】TCP 客户端
│   └── Protocol.h         # 【新增】通信协议定义
├── server/
│   └── main.cpp           # 服务端入口
├── client/
│   └── main.cpp           # 客户端入口
├── src/
│   ├── Server.cpp             # 服务端主逻辑（连接管理+请求分发）
│   ├── UserHandlers.cpp       # 服务端：用户相关命令处理
│   ├── AdminHandlers.cpp      # 服务端：管理员相关命令处理
│   ├── CourierHandlers.cpp    # 服务端：快递员相关命令处理
│   ├── Client.cpp             # 客户端主逻辑（连接+菜单+渲染）
│   ├── DataManager.cpp        # 数据管理
│   ├── Person.cpp / User.cpp / Admin.cpp / Courier.cpp
│   └── Package.cpp / FragilePackage.cpp / BookPackage.cpp / NormalPackage.cpp
├── test_demo.py               # 集成测试脚本
├── concurrency_test.py        # 并发压力测试（10用户×8操作）
├── run_concurrency_test.bat   # 一键编译+测试
└── build.bat                  # 编译脚本
```

### 3.2 核心架构变更：从单机到 C/S

```
题目二（单机）                           题目三（网络版）
                                        
┌─────────────────┐          ┌──────────┐      TCP:8888      ┌──────────┐
│ LogisticsSystem │          │ Client A │────┐           ┌───│ Server   │
│  + DataManager  │          └──────────┘    │           │   ├──────────┤
│  + Controllers  │          ┌──────────┐    ├───────────┤   │DataManager│
│  + Console UI   │          │ Client B │────┘           │   ├──────────┤
└─────────────────┘          └──────────┘                │   │ThreadPool│
                              ┌──────────┐                │   └──────────┘
                              │ Client C │────────────────┘
                              └──────────┘

业务逻辑 + UI   数据         UI 仅在客户端            业务逻辑 + 数据在服务端
全部耦合在一起   文件         客户端是"哑终端"          多线程并发处理
```

**关键设计决策：**
- 客户端：只负责菜单展示和用户输入，不包含任何业务逻辑
- 服务端：持有 `DataManager`，处理所有业务逻辑，线程安全
- 通信协议：纯文本协议，易调试、易扩展

### 3.3 通信协议设计

```
请求格式：COMMAND|param1|param2|...\n
成功响应：OK|message\n[data]\nEND\n
错误响应：ERR|message\n
```

**协议命令全表（约30个）：**

| 分类 | 命令 | 参数 | 说明 |
|------|------|------|------|
| 通用 | `LOGIN` | type, user, pwd | 登录（user/admin/courier） |
| 通用 | `REGISTER` | username, name, phone, pwd, addr | 用户注册 |
| 通用 | `LOGOUT` | — | 退出登录 |
| 通用 | `EXIT` | — | 断开连接 |
| 用户 | `SEND_PACKAGE` | receiver, type, detail, desc | 发送快递 |
| 用户 | `RECEIVE_PACKAGE` | [indices] | 签收快递（两阶段） |
| 用户 | `QUERY_SENT` | — | 查询发出的快递 |
| 用户 | `QUERY_RECEIVED` | — | 查询收到的快递 |
| 用户 | `QUERY_BY_ID` | id | 按单号查询 |
| 用户 | `QUERY_BY_TIME_SENT` | dateType, date | 按寄件时间查询 |
| 用户 | `QUERY_BY_TIME_RECEIVED` | dateType, date | 按签收时间查询 |
| 用户 | `GET_BALANCE` | — | 查询余额 |
| 用户 | `RECHARGE` | amount | 充值 |
| 用户 | `CHANGE_PWD` | oldPwd, newPwd | 修改密码 |
| 管理员 | `ADMIN_SHOW_USERS` | — | 查看所有用户 |
| 管理员 | `ADMIN_SHOW_PACKAGES` | — | 查看所有快递 |
| 管理员 | `ADMIN_SHOW_BALANCE` | — | 查看公司余额 |
| 管理员 | `ADMIN_ADD_COURIER` | name, phone, pwd | 添加快递员 |
| 管理员 | `ADMIN_REMOVE_COURIER` | id | 删除快递员 |
| 管理员 | `ADMIN_SHOW_COURIERS` | — | 查看所有快递员 |
| 管理员 | `ADMIN_SHOW_COURIER_DETAIL` | id | 快递员详情 |
| 管理员 | `ADMIN_ASSIGN_COURIER` | packageId, courierId | 分配揽收任务 |
| 管理员 | `ADMIN_QUERY_BY_SENDER` | sender | 按寄件人查询 |
| 管理员 | `ADMIN_QUERY_BY_RECEIVER` | receiver | 按收件人查询 |
| 管理员 | `ADMIN_QUERY_BY_ID` | id | 按单号查询 |
| 管理员 | `ADMIN_QUERY_BY_TIME` | timeType, dateType, p1, p2 | 按时间查询 |
| 快递员 | `COURIER_MY_TASKS` | — | 我的任务 |
| 快递员 | `COURIER_COLLECT` | [indices] | 揽收（两阶段） |
| 快递员 | `COURIER_MY_RECORDS` | — | 揽收记录 |
| 快递员 | `COURIER_BALANCE` | — | 查询余额 |
| 快递员 | `COURIER_CHANGE_PWD` | oldPwd, newPwd | 修改密码 |

**两阶段协议**：揽收和签收分两步——
1. 客户端发送不带参数的命令 → 服务端返回可操作列表
2. 客户端发送带编号的命令 → 服务端执行操作

### 3.4 服务端设计

#### 请求分发架构

```
                   handleClient (每客户端一个线程)
                          │
                    recv(request)
                          │
                  processRequest()
                    ├── 解析 cmd + params
                    ├── EnterCriticalSection(&cs_)
                    ├── 根据 userType 分发：
                    │   ├── 0 (未登录) → dispatchUnauthCommand()
                    │   ├── 1 (用户)   → dispatchUserCommand()
                    │   ├── 2 (管理员) → dispatchAdminCommand()
                    │   └── 3 (快递员) → dispatchCourierCommand()
                    ├── LeaveCriticalSection(&cs_)
                    └── if (needsSave) → saveData()
                          │
                    send(response)
```

**函数拆分原则**：每个函数 ≤ 100 行。handler 按角色拆分为 3 个文件：
- `UserHandlers.cpp` — 13 个 handler（Login, Register, SendPackage, ReceivePackage 等）
- `AdminHandlers.cpp` — 13 个 handler（ShowUsers, AddCourier, AssignCourier 等）
- `CourierHandlers.cpp` — 5 个 handler（MyTasks, Collect, MyRecords 等）

#### 线程安全设计

```
所有请求处理在 EnterCriticalSection / LeaveCriticalSection 之间执行
  → DataManager 的读写操作串行化
  → 多个客户端同时登录/发快递互不干扰

数据持久化在临界区外执行（needsSave 检查后单独加锁）
  → 文件 I/O 不阻塞其他客户端的业务处理
```

#### 客户端标识

服务端为每个连接分配唯一 ID（`nextClientId_++`），所有日志输出带客户端编号：

```
[客户端#1 连接] 127.0.0.1       ← 新连接，分配 ID
  [#1 请求] zhangsan -> SEND_PACKAGE   ← 显示用户名和命令
  [#1 成功] Package sent! ID: 5        ← 操作结果
  [#1 断开] zhangsan                    ← 断开时显示用户名
```

#### 端口重用

服务端 socket 设置 `SO_REUSEADDR`，允许进程重启后立即绑定端口，无需等待 TIME_WAIT 超时。

### 3.5 客户端设计

客户端是纯粹的"哑终端"：
- `connect()` → `run()` → 主菜单循环 → 子菜单循环
- 每个菜单选项调用对应的 handler 方法
- Handler 方法组装命令字符串 → `sendRequest()` → `receiveResponse()` → `printResponse()`
- 不包含任何业务逻辑，不访问数据文件

**菜单层级**：
```
主菜单
├── 1. 用户登录 → 用户子菜单（查询余额/充值/发快递/收快递/查询/改密）
├── 2. 管理员登录 → 管理员子菜单（用户管理/快递管理/快递员管理/分配任务）
├── 3. 快递员登录 → 快递员子菜单（任务/揽收/记录/余额/改密）
├── 4. 用户注册
└── 0. 退出系统
```

### 3.6 自动化测试

#### 集成测试 (`test_demo.py`)
- 8 个测试用例覆盖主要业务流程
- 使用 Python socket 直接发送协议命令
- 验证成功/失败响应

#### 并发测试 (`concurrency_test.py`)
- 10 个线程同时运行，每个线程执行 8 个操作
- 操作序列：注册 → 登录 → 充值 → 发 3 笔快递 → 查询 → 查余额
- 统计成功率，预期 >90%（部分失败是由于并发竞争导致的合理错误）

#### 一键测试 (`run_concurrency_test.bat`)
```
编译服务端+客户端 → 清理数据 → 启动服务端 → 运行并发测试 → 清理
```

---

## 四、三个题目的延续关系

### 4.1 类体系演进

```
题目一                          题目二                        题目三
                                                          
User (独立) ────→ User : Person ────→ 同题目二，网络传输
                  Person (新增)                           
Admin (独立) ───→ Admin : Person ───→ 同题目二            
                                          
                  Courier : Person ──→ 同题目二            
                  (新增)                                   
                                          
Package (具体) ──→ Package (抽象) ────→ 同题目二            
                    ├── FragilePackage                     
                    ├── BookPackage                        
                    └── NormalPackage                      
```

### 4.2 架构演进

```
题目一              题目二                题目三
──────────         ──────────           ──────────
LogisticsSystem    LogisticsSystem      Server (网络)
  │  │  │            │   │   │            │
  │  │  │            ▼   ▼   ▼            ├── Thread[1..N]
  │  │  │         UserCtrl                │     └── handleClient()
  │  │  │         AdminCtrl               ├── DataManager
  │  │  │         CourierCtrl             └── CriticalSection
  │  │  │            │                        │
  ▼  ▼  ▼            ▼                      Client[1..N]
User Admin Pkg    DataManager                │
(全耦合)          (数据层分离)               ├── sendRequest()
                                           ├── receiveResponse()
                                           └── 中文菜单 UI
```

### 4.3 关键变更对照表

| 维度 | 题目一 | 题目二 | 题目三 |
|------|--------|--------|--------|
| **代码行数(估算)** | ~800 | ~2500 | ~3500 |
| **类数量** | 4 | 14 | 14 |
| **头文件** | 4 | 13 | 12 |
| **源文件** | 5 | 14 | 16 |
| **数据文件** | 4 | 6 | 6（同题目二） |
| **用户角色** | 用户、管理员 | + 快递员 | 同题目二 |
| **快递类型** | 固定 15 元 | 3 种多态计费 | 同题目二 |
| **快递状态** | 0→1 (2种) | 0→1→2 (3种) | 同题目二 |
| **快递员揽收** | 无 | 50%分成 | 同题目二 |
| **数据管理** | 耦合在主类 | DataManager | + 线程安全 |
| **UI 位置** | 混合在业务逻辑 | Controller.showMenu() | 纯客户端 |
| **通信方式** | 无 | 无 | TCP + 文本协议 |
| **线程模型** | 单线程 | 单线程 | 多线程(1 per client) |
| **并发控制** | 无 | 无 | CriticalSection |
| **测试方式** | 手动 | 手动 | Python 自动化 |
| **错误处理** | cin 基本检测 | cin 恢复机制 | + 协议级 + 异常捕获 |

### 4.4 OOP 概念渐进

```
题目一：封装
  ├── private 成员变量 + public getter/setter
  ├── 类内部维护自身数据完整性
  └── 文件持久化隐藏在类内部

题目二：继承 + 多态
  ├── Person 基类消除 User/Admin 重复代码
  ├── Package 纯虚函数 GetPrice() = 0
  ├── dynamic_cast 实现运行时类型识别
  └── vector<Package*> 多态集合存储

题目三：架构 + 并发
  ├── Client-Server 架构分离 UI 与业务逻辑
  ├── 文本协议实现跨进程通信
  ├── CRITICAL_SECTION 实现线程同步
  ├── CreateThread 实现多客户端并发
  └── 自动化测试保证系统正确性
```

---

## 五、编译与运行

### 5.1 环境要求

- **编译器**：MinGW-w64 (g++ 8.0+)，支持 C++11
- **系统**：Windows 10/11
- **库依赖**：`ws2_32`（Winsock2）
- **Python**：3.6+（仅测试脚本需要）

### 5.2 题目一/二编译

```bash
cd topic1  # 或 topic2
g++ -std=c++11 -Wall -o logistics.exe src/main.cpp src/*.cpp -I./include
./logistics.exe
```

### 5.3 题目三编译

```bash
cd topic3

# 编译服务端
g++ -std=c++11 -Wall -o server/server.exe ^
    server/main.cpp src/Server.cpp src/UserHandlers.cpp ^
    src/AdminHandlers.cpp src/CourierHandlers.cpp ^
    src/Person.cpp src/User.cpp src/Admin.cpp src/Courier.cpp ^
    src/Package.cpp src/NormalPackage.cpp src/FragilePackage.cpp ^
    src/BookPackage.cpp src/DataManager.cpp ^
    -I./include -lws2_32

# 编译客户端
g++ -std=c++11 -Wall -o client/client.exe ^
    client/main.cpp src/Client.cpp ^
    -I./include -lws2_32
```

或直接运行 `build.bat`。

### 5.4 题目三运行

```
1. 启动 server/server.exe
2. 启动 client/client.exe（可启动多个）
3. 在客户端进行登录操作
4. 观察服务端控制台的客户端编号日志
```

### 5.5 自动化测试

```bash
cd topic3
run_concurrency_test.bat
```

---

## 六、文件存储兼容性

三个题目的数据文件格式存在差异，体现了系统的演进：

| 文件 | 题目一 | 题目二/三 |
|------|--------|-----------|
| `users.txt` | 6 字段 | 6 字段（相同） |
| `admin.txt` | 4 字段 | 4 字段（相同） |
| `packages.txt` | 7 字段 | 10 字段（增加 type/detail/courierId） |
| `couriers.txt` | 不存在 | 5 字段（新增） |
| `next_id.txt` | 存在 | 存在（相同） |
| `next_courier_id.txt` | 不存在 | 存在（新增） |

题目二和题目三使用完全相同的文件格式，数据文件可以直接共用。

---

## 七、设计原则总结

| 原则 | 体现 |
|------|------|
| **单一职责** | DataManager 只管数据，Controller 只管业务，Client 只管 UI |
| **开闭原则** | Package 子类可扩展（新增快递类型），无需修改现有代码 |
| **里氏替换** | FragilePackage/BookPackage/NormalPackage 可替换 Package 使用 |
| **依赖倒置** | Server/Client 依赖 Protocol 抽象，而非具体实现 |
| **接口隔离** | 每个 Controller 只暴露该角色需要的方法 |

三个题目的递进式设计展示了从**简单的类封装**到**复杂的分布式并发系统**的完整演进过程，是面向对象程序设计教学中的典型案例。
