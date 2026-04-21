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
- 支持文件持久化（CSV 数据存储）
- 支持多管理员管理

---

## 项目结构

```text
LibrarySystem/
├─ src/
│  ├─ main.cpp                 # 程序入口
│  ├─ login/
│  │   ├─ login.cpp            # 登录逻辑实现
│  │   └─ login.h              # 登录头文件
│  ├─ book/
│  │   ├─ book_ui.cpp          # 图书管理界面逻辑
│  │   └─ book_ui.h            # 图书模块头文件
│  ├─ borrow/
│  │   ├─ borrow_ui.cpp        # 借阅界面逻辑
│  │   ├─ borrow_ui.h          # 借阅头文件
│  │   └─ borrow_record.h      # 借阅记录数据结构
│  ├─ reader/
│  │   ├─ reader_ui.cpp        # 读者界面逻辑
│  │   └─ reader_ui.h          # 读者头文件
│  ├─ stat/
│  │   ├─ stat_ui.cpp           # 统计界面逻辑
│  │   └─ stat_ui.h             # 统计头文件
│  └─ utils/
│      └─ ui_utils.h            # 公共 UI 工具类
├─ data/                        # 数据文件（CSV 格式）
│  ├─ admin_accounts.csv        # 管理员账号
│  ├─ books.csv                 # 图书信息
│  ├─ borrow_records.csv        # 借阅记录
│  └─ readers.csv               # 读者信息
├─ picture/                     # 图片资源
├─ README.md
├─ LICENSE
└─ LibrarySystem.sln            # Visual Studio 解决方案文件
