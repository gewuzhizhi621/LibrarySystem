#pragma once

#include <ctime>
#include <sstream>
#include <string>
#include "ui_utils.h"

struct BorrowRecord {
    int bookId = 0;
    std::string bookTitle;
    int readerId = 0;
    std::string readerName;
    std::time_t borrowDate = 0;
    std::time_t dueDate = 0;
    std::time_t returnDate = 0;

    void fromCSV(const std::string& line) {
        std::stringstream ss(line);
        std::string f;

        std::getline(ss, f, ',');
        bookId = f.empty() ? 0 : std::stoi(f);
        std::getline(ss, bookTitle, ',');
        std::getline(ss, f, ',');
        readerId = f.empty() ? 0 : std::stoi(f);
        std::getline(ss, readerName, ',');
        std::getline(ss, f, ',');
        borrowDate = stringToTime(f);
        std::getline(ss, f, ',');
        dueDate = stringToTime(f);
        std::getline(ss, f, ',');
        returnDate = stringToTime(f);
    }

    std::string toCSV() const {
        return std::to_string(bookId) + "," + bookTitle + "," + std::to_string(readerId) + "," + readerName + "," +
            timeToString(borrowDate) + "," + timeToString(dueDate) + "," + timeToString(returnDate);
    }
};
