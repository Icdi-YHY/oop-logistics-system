# -*- coding: utf-8 -*-
import socket
import time
import sys
import io

# 确保输出使用 UTF-8
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

def send_recv(sock, msg):
    sock.send((msg + "\n").encode())
    time.sleep(0.3)
    data = sock.recv(8192).decode().strip()
    lines = data.split("\n")
    for line in lines:
        if line == "END":
            break
        if line.startswith("OK|"):
            print("  [成功] " + line[3:])
        elif line.startswith("ERR|"):
            print("  [错误] " + line[3:])
        else:
            print("   " + line)
    return data

def connect():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", 8888))
    return s

print("=" * 55)
print("  物流管理系统 网络版 - 功能演示")
print("=" * 55)

# ========== TEST 1: 用户注册 ==========
print("\n" + "=" * 55)
print("  测试1: 用户注册")
print("=" * 55)
s1 = connect()
print(">>> 注册用户 alice")
send_recv(s1, "REGISTER|alice|AliceWang|13800001111|alice123|Shanghai")
print(">>> 注册用户 bob")
send_recv(s1, "REGISTER|bob|BobLi|13800002222|bob123|Beijing")
s1.close()

# ========== TEST 2: 用户登录和余额管理 ==========
print("\n" + "=" * 55)
print("  测试2: 用户登录、充值、查询余额")
print("=" * 55)
s1 = connect()
print(">>> alice 登录")
send_recv(s1, "LOGIN|user|alice|alice123")
print(">>> alice 查询余额")
send_recv(s1, "GET_BALANCE")
print(">>> alice 充值 200 元")
send_recv(s1, "RECHARGE|200")
print(">>> alice 再次查询余额")
send_recv(s1, "GET_BALANCE")

# ========== TEST 3: 发送快递 ==========
print("\n" + "=" * 55)
print("  测试3: alice 向 bob 发送快递")
print("=" * 55)
print(">>> 发送易碎品 3kg (8元/kg = 24元) 给 bob")
send_recv(s1, "SEND_PACKAGE|bob|1|3.0|CeramicVase")
print(">>> 发送图书 5本 (2元/本 = 10元) 给 bob")
send_recv(s1, "SEND_PACKAGE|bob|2|5.0|ProgrammingBooks")
print(">>> 发送普通快递 2kg (5元/kg = 10元) 给 bob")
send_recv(s1, "SEND_PACKAGE|bob|3|2.0|Clothes")
print(">>> 查询 alice 发出的快递")
send_recv(s1, "QUERY_SENT")
print(">>> 查询 alice 余额")
send_recv(s1, "GET_BALANCE")
s1.close()

# ========== TEST 4: bob 查看快递 ==========
print("\n" + "=" * 55)
print("  测试4: bob 查看收到的快递")
print("=" * 55)
s2 = connect()
print(">>> bob 登录")
send_recv(s2, "LOGIN|user|bob|bob123")
print(">>> bob 查看收到的快递")
send_recv(s2, "QUERY_RECEIVED")
s2.close()

# ========== TEST 5: 管理员操作 ==========
print("\n" + "=" * 55)
print("  测试5: 管理员登录和快递员管理")
print("=" * 55)
s3 = connect()
print(">>> 管理员登录 (admin / admin123)")
send_recv(s3, "LOGIN|admin|admin|admin123")
print(">>> 查看公司余额")
send_recv(s3, "ADMIN_SHOW_BALANCE")
print(">>> 查看所有用户")
send_recv(s3, "ADMIN_SHOW_USERS")
print(">>> 添加快递员 张三")
send_recv(s3, "ADMIN_ADD_COURIER|ZhangSan|13900001111|zs123")
print(">>> 添加快递员 李四")
send_recv(s3, "ADMIN_ADD_COURIER|LiSi|13900002222|ls123")
print(">>> 查看所有快递员")
send_recv(s3, "ADMIN_SHOW_COURIERS")
print(">>> 查看所有快递")
send_recv(s3, "ADMIN_SHOW_PACKAGES")
s3.close()

# ========== TEST 6: 管理员分配 + 快递员揽收 ==========
print("\n" + "=" * 55)
print("  测试6: 管理员分配快递 + 快递员揽收")
print("=" * 55)
s3 = connect()
print(">>> 管理员登录")
send_recv(s3, "LOGIN|admin|admin|admin123")
print(">>> 分配快递 1 给快递员 1 (ZhangSan)")
send_recv(s3, "ADMIN_ASSIGN_COURIER|1|1")
print(">>> 分配快递 2 给快递员 1 (ZhangSan)")
send_recv(s3, "ADMIN_ASSIGN_COURIER|2|1")
print(">>> 分配快递 3 给快递员 2 (LiSi)")
send_recv(s3, "ADMIN_ASSIGN_COURIER|3|2")
print(">>> 查看公司余额 (分配前)")
send_recv(s3, "ADMIN_SHOW_BALANCE")
s3.close()

# 快递员登录揽收
s4 = connect()
print(">>> 快递员 ZhangSan (ID=1) 登录")
send_recv(s4, "LOGIN|courier|1|zs123")
print(">>> 查看待揽收任务")
send_recv(s4, "COURIER_MY_TASKS")
print(">>> 获取待揽收列表并揽收")
s4.send(b"COURIER_COLLECT|\n")
time.sleep(0.3)
resp = s4.recv(8192).decode().strip()
import re
indices = re.findall(r'^(\d+)\|', resp, re.MULTILINE)
print("  待揽收索引: " + str(indices))
if indices:
    idx_list = ",".join(indices)
    print(">>> 揽收快递: " + idx_list)
    s4.send(("COURIER_COLLECT|" + idx_list + "\n").encode())
    time.sleep(0.3)
    r = s4.recv(8192).decode().strip()
    for line in r.split("\n"):
        if line == "END":
            break
        if line.startswith("OK|"):
            print("  [成功] " + line[3:])
        elif line.startswith("ERR|"):
            print("  [错误] " + line[3:])
        else:
            print("   " + line)

print(">>> 查看快递员余额")
send_recv(s4, "COURIER_BALANCE")
print(">>> 查看揽收记录")
send_recv(s4, "COURIER_MY_RECORDS")
s4.close()

# 管理员查看最终状态
s5 = connect()
print(">>> 管理员查看最终公司余额")
send_recv(s5, "LOGIN|admin|admin|admin123")
send_recv(s5, "ADMIN_SHOW_BALANCE")
print(">>> 查看快递员 1 详情")
send_recv(s5, "ADMIN_SHOW_COURIER_DETAIL|1")
s5.close()

# ========== TEST 7: bob签收快递 ==========
print("\n" + "=" * 55)
print("  测试7: bob签收已揽收的快递")
print("=" * 55)
s6 = connect()
print(">>> bob 登录")
send_recv(s6, "LOGIN|user|bob|bob123")
print(">>> bob 查看待签收快递")
s6.send(b"QUERY_RECEIVED\n")
time.sleep(0.3)
resp = s6.recv(8192).decode().strip()
print(resp)
# 获取待签收列表
s6.send(b"RECEIVE_PACKAGE|\n")
time.sleep(0.3)
resp = s6.recv(8192).decode().strip()
print("  响应内容:")
for line in resp.split("\n"):
    if line == "END":
        break
    if line.startswith("OK|"):
        print("  [成功] " + line[3:])
    elif line.startswith("ERR|"):
        print("  [错误] " + line[3:])
    else:
        print("   " + line)
indices = re.findall(r'^(\d+)\|', resp, re.MULTILINE)
print("  可签收快递索引: " + str(indices))
if indices:
    idx_list = ",".join(indices)
    print(">>> 签收快递: " + idx_list)
    s6.send(("RECEIVE_PACKAGE|" + idx_list + "\n").encode())
    time.sleep(0.3)
    r = s6.recv(8192).decode().strip()
    for line in r.split("\n"):
        if line == "END":
            break
        if line.startswith("OK|"):
            print("  [成功] " + line[3:])
        elif line.startswith("ERR|"):
            print("  [错误] " + line[3:])
        else:
            print("   " + line)
print(">>> bob 再次查看收到的快递")
send_recv(s6, "QUERY_RECEIVED")
s6.close()

# ========== TEST 8: 错误处理测试 ==========
print("\n" + "=" * 55)
print("  测试8: 错误场景处理验证")
print("=" * 55)
s7 = connect()
print(">>> 测试错误密码登录")
send_recv(s7, "LOGIN|user|alice|wrongpassword")
print(">>> 测试不存在的用户")
send_recv(s7, "LOGIN|user|nobody|123456")
print(">>> 测试未登录直接操作")
send_recv(s7, "GET_BALANCE")
print(">>> alice 正确登录")
send_recv(s7, "LOGIN|user|alice|alice123")
print(">>> 测试余额不足发送快递 (余额156元, 尝试发送100kg易碎品)")
send_recv(s7, "SEND_PACKAGE|bob|1|100.0|TooExpensive")
print(">>> 测试发送给不存在的用户")
send_recv(s7, "SEND_PACKAGE|nobody|3|1.0|Test")
s7.close()

print("\n" + "=" * 55)
print("  全部测试完成! CS架构物流系统运行正常!")
print("=" * 55)
