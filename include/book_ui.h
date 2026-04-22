#pragma once
#include <string>
#include <sstream>
#include <vector>

// 图书数据结构（供其他模块使用）
struct Book {
    int id = 0;
    std::string title;
    std::string author;
    std::string publisher;
    int stock = 0;
    int borrowed = 0;

    void fromCSV(const std::string& line) {
        std::stringstream ss(line);
        std::string fld;
        std::getline(ss, fld, ','); id = std::stoi(fld);
        std::getline(ss, title, ',');
        std::getline(ss, author, ',');
        std::getline(ss, publisher, ',');
        std::getline(ss, fld, ','); stock = std::stoi(fld);
        std::getline(ss, fld, ','); borrowed = std::stoi(fld);
    }

    std::string toCSV() const {
        return std::to_string(id) + "," + title + "," + author + "," + publisher + "," +
            std::to_string(stock) + "," + std::to_string(borrowed);
    }
};

// 对外可见的数据与函数
extern std::vector<Book> g_books;
void LoadBooks();
void SaveBooks();

// 图书管理界面入口
void bookUIMain();