#pragma once
#include <string>
#include <sstream>
#include <vector>

struct Reader {
    int id = 0;
    std::string name;
    std::string contact;
    std::string regDate;

    void fromCSV(const std::string& line) {
        std::stringstream ss(line);
        std::string f;
        std::getline(ss, f, ','); id = std::stoi(f);
        std::getline(ss, name, ',');
        std::getline(ss, contact, ',');
        std::getline(ss, regDate, ',');
    }

    std::string toCSV() const {
        return std::to_string(id) + "," + name + "," + contact + "," + regDate;
    }
};

// 对外可见的数据与函数
extern std::vector<Reader> g_readers;
void LoadReaders();
void SaveReaders();

void readerUIMain();