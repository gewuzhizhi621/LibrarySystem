#pragma once

#include <sstream>
#include <string>
#include <vector>

struct Book {
    int id = 0;
    std::string title;
    std::string author;
    std::string publisher;
    std::string category;
    int stock = 0;
    int borrowed = 0;
    int totalBorrowed = 0;

    void fromCSV(const std::string& line) {
        std::stringstream ss(line);
        std::string f;

        std::getline(ss, f, ',');
        id = f.empty() ? 0 : std::stoi(f);
        std::getline(ss, title, ',');
        std::getline(ss, author, ',');
        std::getline(ss, publisher, ',');
        std::getline(ss, category, ',');
        std::getline(ss, f, ',');
        stock = f.empty() ? 0 : std::stoi(f);
        std::getline(ss, f, ',');
        borrowed = f.empty() ? 0 : std::stoi(f);
        std::getline(ss, f, ',');
        totalBorrowed = f.empty() ? 0 : std::stoi(f);
    }

    std::string toCSV() const {
        return std::to_string(id) + "," + title + "," + author + "," + publisher + "," + category + "," +
            std::to_string(stock) + "," + std::to_string(borrowed) + "," + std::to_string(totalBorrowed);
    }
};

extern std::vector<Book> g_books;

void LoadBooks();
void SaveBooks();
void bookUIMain();
void bookQueryUIMain();
