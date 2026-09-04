# 东软电动汽车充电桩应用管理平台

基于 Qt/C++ 开发的充电桩管理系统，包含 **充电用户端** 和 **PC 服务器端** 两个程序。

## 项目结构

```
充电桩项目/
├── common/                # 共享协议库（帧封包/拆包 + 数据类型）
│   ├── protocol.h/.cpp    # 通信协议：帧头(Magic+MsgId+Length) + JSON Payload
│   ├── datatypes.h        # UserInfo / StationInfo / PileInfo / OrderInfo
│   └── common.pri         # qmake include 文件
├── server/                # PC 服务器端（管理平台）
│   ├── server.pro
│   ├── main.cpp           # 多线程启动（业务线程 + UI 主线程）
│   ├── database.*         # SQLite 数据库管理（建表/种子/CRUD）
│   ├── serverservice.*    # 核心服务：TCP + 充电业务 + 模拟充电 + 断线结算
│   ├── adminloginwidget.* # 管理员登录
│   ├── mainwindow.*       # 主界面（TabWidget）
│   ├── salespage.*        # 销售业绩（Qt Charts 营收趋势图）
│   ├── pilestatuspage.*   # 电桩状态分布
│   ├── pilemanagepage.*   # 充电桩管理（远程重启）
│   ├── stationmanagepage.*# 充电站管理（新增/详情）
│   ├── usermanagepage.*   # 用户管理（冻结/解冻/搜索）
│   └── orderspage.*       # 订单记录
├── client/                # 充电用户端（竖屏手机风格）
│   ├── client.pro
│   ├── main.cpp
│   ├── tcpclient.*        # TCP 客户端 + 心跳
│   ├── mainwindow.*       # 主窗口（QStackedWidget 页面切换）
│   ├── loginpage.*        # 登录页（手机号免密登录）
│   ├── stationlistpage.*  # 充电站列表（定位 + 距离排序）
│   ├── piledetailpage.*   # 电桩详情（选择空闲桩充电）
│   ├── chargepage.*       # 充电页（实时数据 + 功率曲线 + 结算）
│   ├── profilepage.*      # 我的（信息/充值/订单）
│   └── mapview.*          # 地图导航（QWebEngineView + 腾讯地图）
└── 项目文档/
    ├── 开发文档.md
    └── README.md          # 本文件
```

## 环境要求

- **OS**: Linux（Ubuntu 22.04 推荐）/ Windows / macOS
- **Qt**: 6.2+（需包含 Charts、WebEngine、Sql、Network 模块）
- **编译器**: GCC 9+ / MSVC 2019+ / Clang 12+

### Qt 模块依赖

| 模块 | 用途 | server | client |
|------|------|--------|--------|
| core gui widgets | 基础 UI | ✓ | ✓ |
| network | TCP 通信 | ✓ | ✓ |
| sql | SQLite 数据库 | ✓ | - |
| charts | 图表（营收/功率曲线） | ✓ | ✓ |
| webenginewidgets | 地图导航 | - | ✓ |

## 编译与运行

### 方式一：Qt Creator（推荐）

1. 打开 Qt Creator
2. 分别打开 `server/server.pro` 和 `client/client.pro`
3. 点击 **构建 → 运行**（Ctrl+R）

### 方式二：命令行

```bash
# 编译服务器端
cd server
qmake server.pro
make -j$(nproc)
./charging_server

# 编译用户端（另开终端）
cd client
qmake client.pro
make -j$(nproc)
./charging_client
```

## 运行说明

### 服务器端

1. 启动后先弹出 **管理员登录** 对话框
   - 默认账号：`admin`
   - 默认密码：`123456`
2. 登录成功后进入主界面，自动开始监听 TCP 端口 **9527**
3. 主界面包含 6 个功能页签：
   - **销售业绩**：今日/本月/总营收 + 营收趋势折线图
   - **电桩状态**：全平台电桩状态分布（空闲/充电中/故障/离线）
   - **充电桩管理**：电桩列表 + 远程重启
   - **充电站管理**：充电站列表 + 新增 + 站内电桩详情
   - **用户管理**：用户列表 + 搜索 + 冻结/解冻
   - **订单记录**：全部充电订单

### 用户端

1. 启动后进入登录页，输入 11 位手机号免密登录（首次自动注册）
2. 可配置服务器地址和端口（默认 `127.0.0.1:9527`）
3. 登录后进入充电站列表：
   - 选择区域或手动输入地址定位
   - 列表按距离由近及远排序
   - 点击 **详情** 查看站内电桩
   - 点击 **导航** 打开地图路线规划
4. 在电桩详情页点击空闲电桩开始充电
5. 充电页实时显示电压/电流/功率/SOC/电量 + 功率曲线
6. 点击 **停止充电并结算** 或 SOC 充满自动结算
7. **我的** 页可修改昵称/头像、充值、查看订单

### 数据库

- SQLite 数据库文件 `charging_platform.db` 自动创建在服务器可执行文件同目录
- 首次启动自动注入种子数据：6 个充电站、36 个电桩、1 个管理员
- 密码使用 SHA-256 哈希存储

## 通信协议

帧格式（大端字节序）：

```
+----------+----------+----------+---------------------+
|  Magic   |  MsgId   |  Length  |       Payload       |
|  2 字节  |  2 字节  |  4 字节  |   Length 字节       |
+----------+----------+----------+---------------------+
```

- **Magic**: `0x4E51`（'NQ'），帧起始校验
- **MsgId**: 消息类型（1xxx 请求 / 2xxx 响应）
- **Length**: Payload 字节长度（解决 TCP 粘包/半包）
- **Payload**: UTF-8 编码的 JSON 文本

## 架构特点

- **多线程**：服务器端 ServerService 运行在独立业务线程，UI 在主线程，通过信号槽通信
- **断线自动结算**：客户端充电中断线时服务器自动结算，避免电桩永久占用
- **心跳机制**：客户端每 30 秒发送心跳，服务器 90 秒无数据判定离线
- **模拟充电**：QTimer 每秒推进，模拟电压/电流/功率/SOC/电量（快充 500V/100A，慢充 220V/16A）
- **地图导航**：QWebEngineView 加载腾讯地图 GL JS API，DirectionService 驾车路线规划