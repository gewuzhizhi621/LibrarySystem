# LibrarySystem 图书管理系统

图书管理系统项目，使用 C++ 开发，适用于大一同学课设参考。  
该系统可以管理图书信息，包括添加、删除、查询和修改图书记录。

---

## 功能

- 用户登录
- 图书管理（添加、删除、修改、查询）
- 借阅管理（借书、还书、记录查询）
- 读者管理
- 统计报表显示
- 支持文件持久化（数据存储到本地文件）

---

## 项目结构

LibrarySystem/
├─ src/
│  ├─ main.cpp                 # 程序入口
│  ├─ login/
│  │   ├─ login.cpp
│  │   └─ login.h
│  ├─ book/
│  │   ├─ book_ui.cpp
│  │   └─ book_ui.h
│  ├─ borrow/
│  │   ├─ borrow_ui.cpp
│  │   ├─ borrow_ui.h
│  │   └─ borrow_record.h
│  ├─ reader/
│  │   ├─ reader_ui.cpp
│  │   └─ reader_ui.h
│  ├─ stat/
│  │   ├─ stat_ui.cpp
│  │   └─ stat_ui.h
│  └─ utils/
│      └─ ui_utils.h
├─ data/                        # 数据文件（可选）
├─ picture/                     # 图片资源
├─ README.md
├─ LICENSE
└─ LibrarySystem.sln            # Visual Studio 解决方案文件
