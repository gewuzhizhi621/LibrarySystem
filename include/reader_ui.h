#pragma once

#include <sstream>
#include <string>
#include <vector>

struct Reader {
    int id = 0;
    std::string name;
    std::string contact;
    std::string regDate;
    std::string username;
    std::string password;
    std::string status;

    void fromCSV(const std::string& line) {
        std::stringstream ss(line);
        std::string f;

        std::getline(ss, f, ',');
        id = f.empty() ? 0 : std::stoi(f);
        std::getline(ss, name, ',');
        std::getline(ss, contact, ',');
        std::getline(ss, regDate, ',');
        std::getline(ss, username, ',');
        std::getline(ss, password, ',');
        std::getline(ss, status, ',');

        if (status.empty()) {
            status = "normal";
        }
    }

    std::string toCSV() const {
        return std::to_string(id) + "," + name + "," + contact + "," + regDate + "," + username + "," + password + "," + status;
    }
};

extern std::vector<Reader> g_readers;

void LoadReaders();
void SaveReaders();
void readerUIMain();
void ChangePasswordUI(int readerId);
