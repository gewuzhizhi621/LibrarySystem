# LibrarySystem 图书管理系统

---

## 项目简介

**LibrarySystem 图书管理系统** 是一个基于 **C++ + EasyX 图形库** 开发的图形化图书管理系统。

系统支持 **管理员端** 和 **读者端** 两种身份登录。管理员可以完成图书管理、借阅管理、读者管理、数据统计、操作日志查看和数据备份等功能；读者可以查询图书、查看个人借阅记录以及修改密码。

项目数据采用 **CSV 文件** 进行本地存储，不依赖数据库，结构清晰、易于维护，适合作为 **C++ 课程设计、大一程序设计项目、EasyX 图形界面练习项目** 使用。

---

## 项目界面展示

<table>
  <tr>
    <td align="center" width="50%">
      <b>登录界面</b><br>
      <img src="docs/images/login.png" width="480">
    </td>
    <td align="center" width="50%">
      <b>管理员主界面</b><br>
      <img src="docs/images/admin_menu.png" width="480">
    </td>
  </tr>
  
  <tr>
    <td align="center" width="50%">
      <b>图书管理</b><br>
      <img src="docs/images/book_manage.png" width="480">
    </td>
    <td align="center" width="50%">
      <b>借阅管理</b><br>
      <img src="docs/images/borrow_manage.png" width="480">
    </td>
  </tr>
  
  <tr>
    <td align="center" width="50%">
      <b>读者管理</b><br>
      <img src="docs/images/reader_manage.png" width="480">
    </td>
    <td align="center" width="50%">
      <b>数据统计中心</b><br>
      <img src="docs/images/statistics.png" width="480">
    </td>
  </tr>
  
  <tr>
    <td align="center" width="50%">
      <b>操作日志</b><br>
      <img src="docs/images/logs.png" width="480">
    </td>
    <td align="center" width="50%">
      <b>读者服务中心</b><br>
      <img src="docs/images/reader_menu.png" width="480">
    </td>
  </tr>
</table>

---

## 功能介绍

### 用户登录

- **登录身份：** 管理员账号登录、读者账号登录、管理员 / 读者身份切换
- **登录验证：** 基于本地 CSV 文件验证账号密码，登录失败时给出错误提示
- **登录提醒：** 读者登录后自动检测是否存在逾期借阅记录

---

### 图书管理

- **基础操作：** 添加图书、删除图书、修改图书信息、查询图书
- **辅助功能：** 分类筛选、排序显示、分页浏览、低库存红色提示
- **图书字段：** 图书 ID、书名、作者、出版社、分类、库存数量、当前借出数量、累计借阅次数

---

### 读者管理

- **基础操作：** 添加读者、删除读者、修改读者信息、查询读者
- **账号管理：** 查看账号状态、正常账号管理、停用账号管理、分页浏览
- **读者字段：** 读者 ID、姓名、联系方式、注册日期、登录账号、登录密码、账号状态

---

### 借阅管理

- **基础操作：** 借书、还书、续借、查询借阅记录
- **记录筛选：** 只看未还、只看逾期、逾期红色提示、分页浏览
- **借阅规则：** 图书不存在不能借阅、读者不存在不能借阅、停用账号不能借阅、库存不足不能借阅
- **限制规则：** 逾期未还不能继续借阅、最多同时借阅 5 本、默认借阅 14 天、续借延长 7 天

---

### 数据统计

- **基础统计：** 图书种类统计、库存总量统计、读者总数统计
- **借阅统计：** 当前借出数量统计、当前逾期数量统计
- **图形展示：** 热门图书 TOP 5、柱状图展示、逾期清单展示

---

### 操作日志

- **日志内容：** 操作时间、操作用户、用户身份、操作类型、操作详情
- **记录类型：** 登录系统、添加图书、修改图书、删除图书、添加读者、修改读者、删除读者
- **借阅日志：** 借书、还书、续借、查看日志

---

### 数据备份

- **备份功能：** 一键备份 CSV 数据文件、自动创建 backup 目录、便于恢复历史数据

---

## 开发环境

| 配置项 | 编程语言 | C++ 标准 | 开发工具 | 图形库 | 数据存储 | 操作系统 | 推荐平台 | 推荐字符集 |
|---|---|---|---|---|---|---|---|---|
| 说明 | C++ | C++17 | Visual Studio 2022 | EasyX | CSV 文件 | Windows 10 / Windows 11 | x64 | Unicode |

---

## 项目结构

```text
LibrarySystem/
│
├── src/                         # 源代码文件
│   ├── main.cpp                 # 程序入口与主菜单
│   ├── login.cpp                # 登录界面与登录验证
│   ├── book_ui.cpp              # 图书管理界面
│   ├── borrow_ui.cpp            # 借阅管理界面
│   ├── reader_ui.cpp            # 读者管理界面
│   ├── stat_ui.cpp              # 数据统计界面
│   └── log_ui.cpp               # 操作日志界面
│
├── include/                     # 头文件
│   ├── login.h                  # 登录模块声明
│   ├── book_ui.h                # 图书管理模块声明
│   ├── borrow_ui.h              # 借阅管理模块声明
│   ├── borrow_record.h          # 借阅记录结构体
│   ├── reader_ui.h              # 读者管理模块声明
│   ├── stat_ui.h                # 数据统计模块声明
│   ├── log_ui.h                 # 操作日志模块声明
│   └── ui_utils.h               # 公共 UI 工具函数
│
├── data/                        # CSV 数据文件
│   ├── admin_accounts.csv       # 管理员账号数据
│   ├── books.csv                # 图书信息数据
│   ├── readers.csv              # 读者信息数据
│   ├── borrow_records.csv       # 借阅记录数据
│   └── logs.csv                 # 操作日志数据
│
├── picture/                     # 程序背景图片资源
│
├── Easyx/                       # EasyX 图形库
│
├── docs/                        # README 展示图片
│   └── images/
│
├── backup/                      # 数据备份目录，运行后自动生成
│
├── LibrarySystem.sln            # Visual Studio 解决方案文件
├── LibrarySystem.vcxproj        # Visual Studio 项目文件
└── README.md                    # 项目说明文档
```

---

## 数据文件说明

系统数据统一存储在 `data/` 目录下，采用 CSV 文件进行本地持久化保存，便于查看、修改和备份。

| 文件名 | `admin_accounts.csv` | `books.csv` | `readers.csv` | `borrow_records.csv` | `logs.csv` |
|---|---|---|---|---|---|
| 说明 | 管理员账号数据 | 图书信息数据 | 读者信息数据 | 借阅记录数据 | 操作日志数据 |

---

## CSV 字段说明

### admin_accounts.csv

```csv
username,password,name,role
```

| 字段 | username | password | name | role |
|---|---|---|---|---|
| 说明 | 管理员账号 | 管理员密码 | 管理员名称 | 用户角色 |

---

### books.csv

```csv
id,title,author,publisher,category,stock,borrowed,totalBorrowed
```

| 字段 | id | title | author | publisher | category | stock | borrowed | totalBorrowed |
|---|---|---|---|---|---|---|---|---|
| 说明 | 图书编号 | 书名 | 作者 | 出版社 | 图书分类 | 库存总数 | 当前借出数量 | 累计借阅次数 |

---

### readers.csv

```csv
id,name,contact,regDate,username,password,status
```

| 字段 | id | name | contact | regDate | username | password | status |
|---|---|---|---|---|---|---|---|
| 说明 | 读者编号 | 读者姓名 | 联系方式 | 注册日期 | 登录账号 | 登录密码 | 账号状态 |

账号状态说明：

| 状态 | normal | disabled |
|---|---|---|
| 含义 | 正常账号 | 停用账号 |

---

### borrow_records.csv

```csv
bookId,bookTitle,readerId,readerName,borrowDate,dueDate,returnDate
```

| 字段 | bookId | bookTitle | readerId | readerName | borrowDate | dueDate | returnDate |
|---|---|---|---|---|---|---|---|
| 说明 | 图书编号 | 图书名称 | 读者编号 | 读者姓名 | 借书日期 | 应还日期 | 归还日期，空值表示未归还 |

---

### logs.csv

```csv
time,user,role,action,detail
```

| 字段 | time | user | role | action | detail |
|---|---|---|---|---|---|
| 说明 | 操作时间 | 操作用户 | 用户身份 | 操作类型 | 操作详情 |
---

## 运行方法

### 方法一：Visual Studio 2022 运行

| 步骤 | 操作 |
|---|---|
| 1 | 使用 **Visual Studio 2022** 打开 `LibrarySystem.sln` |
| 2 | 将平台设置为 `x64` |
| 3 | 将字符集设置为 `Unicode` |
| 4 | 确保已配置 EasyX 头文件和库文件路径 |

---

### 方法二：项目自带 EasyX 库运行

如果项目中已经包含 `Easyx/` 文件夹，需要确认以下配置正确。

| 配置项 | 内容 |
|---|---|
| 附加包含目录 | `$(ProjectDir)include` |
| 附加包含目录 | `$(ProjectDir)Easyx\include` |
| 附加库目录 | `$(ProjectDir)Easyx\lib\VC2015\X64` |
| 附加依赖项 | `EasyXw.lib` |

---

## 作者

如果本项目对你有帮助，欢迎 Star ⭐
