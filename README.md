# 📚 LibrarySystem 图书管理系统

## 📖 项目简介

本项目是基于 C++ 开发的简易图书管理系统，结合 EasyX 图形库实现图形化界面，适用于大一课程设计或基础项目练习。

系统支持图书管理、读者管理、借阅管理及数据统计等功能，数据以 CSV 文件形式存储，结构清晰、易于维护。

---

## 🚀 功能介绍

### 👤 用户登录

* 管理员账号登录
* 基于本地 CSV 数据验证

### 📚 图书管理

* 添加图书
* 删除图书
* 查询图书
* 修改图书信息

### 📖 读者管理

* 添加读者
* 删除读者
* 查询读者

### 🔄 借阅管理

* 借书
* 还书
* 借阅记录查询

### 📊 数据统计

* 借阅情况统计
* 数据展示（图形界面）

---

## 🛠️ 开发环境

* 编程语言：C++
* 开发工具：Visual Studio 2022
* 图形库：EasyX
* 操作系统：Windows

---

## 📂 项目结构

```
LibrarySystem/
│
├── src/                # 源代码（.cpp）
│   ├── main.cpp
│   ├── login.cpp
│   ├── book_ui.cpp
│   ├── reader_ui.cpp
│   ├── borrow_ui.cpp
│   └── stat_ui.cpp
│
├── include/            # 头文件（.h）
│   ├── login.h
│   ├── book_ui.h
│   ├── reader_ui.h
│   ├── borrow_record.h
│   ├── stat_ui.h
│   └── ui_utils.h
│
├── data/               # 数据文件（CSV）
│   ├── books.csv
│   ├── readers.csv
│   ├── borrow_records.csv
│   └── admin_accounts.csv
│
├── picture/            # 图片资源
│   ├── bgImg.png
│   └── logoImg.jpg
│
├── Easyx/              # EasyX库
│
├── LibrarySystem.sln   # VS解决方案文件
├── LibrarySystem.vcxproj
└── README.md
```

---

## ⚙️ 运行方法

### 方法一（推荐）

1. 使用 **Visual Studio 2022** 打开 `LibrarySystem.sln`
2. 确保已安装 EasyX 图形库
3. 直接运行项目（F5）

---

### 方法二（无需安装 EasyX）

如果项目中已包含 `Easyx/` 文件夹：

1. 确保项目已正确配置库路径
2. 直接运行即可

---

## 📊 数据说明

所有数据存储在 `data/` 目录下的 CSV 文件中：

| 文件名                | 说明    |
| ------------------ | ----- |
| books.csv          | 图书信息  |
| readers.csv        | 读者信息  |
| borrow_records.csv | 借阅记录  |
| admin_accounts.csv | 管理员账号 |

---

## 📌 项目特点

* 使用 C++ 实现完整业务逻辑
* 基于 EasyX 实现图形界面
* 使用 CSV 作为轻量级数据库
* 模块化设计（UI + 数据分离）
* 易扩展、适合学习和课程设计

---

## ⚠️ 注意事项

* 请确保 EasyX 已正确安装或项目包含相关库
* 程序依赖本地 CSV 文件，请勿删除 `data/` 目录
* 建议使用 x64 平台运行

---


---
