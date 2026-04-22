#pragma once
#include <string>
#include <ctime>

struct BorrowRecord {
    int bookId = 0;
    std::string bookTitle;
    int readerId = 0;
    std::string readerName;
    std::time_t borrowDate = 0;
    std::time_t dueDate = 0;
    std::time_t returnDate = 0;

    void fromCSV(const std::string& line);
    std::string toCSV() const;
};