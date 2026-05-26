# -*- coding: utf-8 -*-
"""
物流管理系统 — 多用户并发模拟测试
模拟 N 个用户同时登录、发送快递、查询等操作，验证多线程服务端稳定性
"""
import socket
import threading
import time
import sys
import random

HOST = "127.0.0.1"
PORT = 8888
USERS = 10          # 模拟用户数
OPS_PER_USER = 8    # 每用户操作数

results = []
lock = threading.Lock()

def log(uid, msg):
    with lock:
        print(f"[用户{uid:02d}] {msg}")

def user_session(uid):
    """单个用户模拟：注册 → 登录 → 充值 → 发快递 → 查询"""
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(10)
    try:
        s.connect((HOST, PORT))
    except Exception as e:
        log(uid, f"连接失败: {e}")
        return False

    def send_recv(msg):
        try:
            s.send((msg + "\n").encode())
            time.sleep(0.05)
            return s.recv(8192).decode().strip()
        except:
            return "ERR|网络异常"

    ok_count = 0
    uname = f"user{uid}"
    name  = f"测试用户{uid}"
    phone = f"1380000{uid:04d}"
    pwd   = f"pass{uid}"
    addr  = f"地址{uid}号"

    # 1. 注册
    resp = send_recv(f"REGISTER|{uname}|{name}|{phone}|{pwd}|{addr}")
    if resp.startswith("OK"): ok_count += 1
    log(uid, f"注册: {resp[:50]}")

    # 2. 登录
    resp = send_recv(f"LOGIN|user|{uname}|{pwd}")
    if resp.startswith("OK"): ok_count += 1
    log(uid, f"登录: {resp[:50]}")

    # 3. 充值
    amount = random.randint(50, 200)
    resp = send_recv(f"RECHARGE|{amount}")
    if resp.startswith("OK"): ok_count += 1
    log(uid, f"充值{amount}元: {resp[:50]}")

    # 4-6. 发 3 笔快递给不同用户
    receivers = [f"user{r}" for r in random.sample(range(USERS), min(3, USERS-1)) if r != uid]
    for i, recv in enumerate(receivers[:3]):
        ptype = random.choice([1, 2, 3])
        if ptype == 1:
            detail = f"{random.uniform(0.5, 5.0):.1f}"
        elif ptype == 2:
            detail = str(random.randint(1, 5))
        else:
            detail = f"{random.uniform(0.5, 5.0):.1f}"
        desc = f"物品{uid}-{i}"
        resp = send_recv(f"SEND_PACKAGE|{recv}|{ptype}|{detail}|{desc}")
        if resp.startswith("OK"): ok_count += 1
        log(uid, f"发快递{i+1}→{recv}: {resp[:60]}")

    # 7. 查询发出的快递
    resp = send_recv("QUERY_SENT")
    if resp.startswith("OK"): ok_count += 1
    log(uid, f"查询发出: {'OK' if resp.startswith('OK') else 'FAIL'}")

    # 8. 查询余额
    resp = send_recv("GET_BALANCE")
    if resp.startswith("OK"): ok_count += 1
    log(uid, f"查余额: {resp[:50]}")

    s.close()

    with lock:
        results.append((uid, ok_count, OPS_PER_USER))

    log(uid, f"完成: {ok_count}/{OPS_PER_USER} 操作成功")
    return True

def main():
    print("=" * 55)
    print(f"  物流系统并发测试 — {USERS} 用户同时操作")
    print("=" * 55)

    start = time.time()
    threads = []
    for uid in range(USERS):
        t = threading.Thread(target=user_session, args=(uid,))
        threads.append(t)
        t.start()
        time.sleep(0.02)  # 微间隔，避免瞬间全连

    for t in threads:
        t.join()

    elapsed = time.time() - start

    # 统计
    total_ok = sum(r[1] for r in results)
    total_ops = sum(r[2] for r in results)
    failed_users = [r[0] for r in results if r[1] < r[2]]

    print("\n" + "=" * 55)
    print(f"  并发测试完成")
    print(f"  总用户: {USERS} | 总操作: {total_ops} | 成功: {total_ok}")
    print(f"  成功率: {total_ok / total_ops * 100:.1f}%")
    print(f"  耗时: {elapsed:.2f}s")
    if failed_users:
        print(f"  部分失败用户: {failed_users}")
    else:
        print(f"  全部用户操作正常!")
    print("=" * 55)

if __name__ == "__main__":
    main()
