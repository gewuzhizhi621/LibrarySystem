#include "borrow_ui.h"
#include "ui_utils.h"
#include "borrow_record.h"
#include "book_ui.h"          // 使用 Book 结构体、g_books、LoadBooks、SaveBooks
#include "reader_ui.h"        // 使用 Reader 结构体、g_readers、LoadReaders、SaveReaders
#include <chrono>
#include <algorithm>

// ---------- 借阅记录全局数据 ----------
std::vector<BorrowRecord> g_borrows;
static int page = 0, totalPage = 0;
static int selectedRow = -1;

// ---------- BorrowRecord 成员函数实现 ----------
void BorrowRecord::fromCSV(const std::string& line) {
    std::stringstream ss(line);
    std::string f;
    std::getline(ss, f, ','); bookId = std::stoi(f);
    std::getline(ss, bookTitle, ',');
    std::getline(ss, f, ','); readerId = std::stoi(f);
    std::getline(ss, readerName, ',');
    std::getline(ss, f, ','); borrowDate = stringToTime(f);
    std::getline(ss, f, ','); dueDate = stringToTime(f);
    std::getline(ss, f, ','); returnDate = stringToTime(f);
}

std::string BorrowRecord::toCSV() const {
    return std::to_string(bookId) + "," + bookTitle + "," + std::to_string(readerId) + "," +
        readerName + "," + timeToString(borrowDate) + "," + timeToString(dueDate) + "," +
        timeToString(returnDate);
}

// ---------- 数据加载与保存 ----------
void ReloadBorrows() {
    LoadCSV("data/borrow_records.csv", g_borrows, true);
    totalPage = (g_borrows.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    if (totalPage == 0) totalPage = 1;
}

void SaveBorrows() {
    SaveCSV("data/borrow_records.csv", g_borrows, "bookId,bookTitle,readerId,readerName,borrowDate,dueDate,returnDate");
}

// ---------- 绘制借阅表格 ----------
static void DrawBorrowTable(int x0, int y0) {
    std::vector<std::wstring> headers = { L"ID", L"书名", L"读者", L"借书日", L"应还日", L"归还日" };
    std::vector<int> widths = { 50, 200, 150, 120, 120, 120 };
    DrawTableHeader(x0, y0, headers, widths);

    int start = page * PAGE_SIZE;
    for (int i = 0; i < PAGE_SIZE && start + i < (int)g_borrows.size(); ++i) {
        const BorrowRecord& br = g_borrows[start + i];
        int y = y0 + ROW_H * (i + 1);
        COLORREF rowBg = (selectedRow == start + i) ? RGB(220, 240, 255) : WHITE;
        int curX = x0;
        for (size_t j = 0; j < widths.size(); ++j) {
            DrawRoundedRect(curX, y, curX + widths[j], y + ROW_H, 0, rowBg, CLR_TEXT_LIGHT);
            curX += widths[j];
        }
        setbkmode(TRANSPARENT);
        settextcolor(CLR_TEXT_DARK);
        settextstyle(18, 0, L"微软雅黑");
        curX = x0;
        outtextxy(curX + 5, y + 8, std::to_wstring(start + i + 1).c_str()); curX += widths[0];
        outtextxy(curX + 5, y + 8, s2ws(br.bookTitle).c_str()); curX += widths[1];
        outtextxy(curX + 5, y + 8, s2ws(br.readerName).c_str()); curX += widths[2];
        outtextxy(curX + 5, y + 8, timeToWstring(br.borrowDate).c_str()); curX += widths[3];
        outtextxy(curX + 5, y + 8, timeToWstring(br.dueDate).c_str()); curX += widths[4];
        outtextxy(curX + 5, y + 8, timeToWstring(br.returnDate).c_str());
    }
}

// ---------- 借书对话框 ----------
static void BorrowDialog() {
    LoadBooks();      // 确保图书数据最新
    LoadReaders();    // 确保读者数据最新

    std::wstring bookIdStr, readerIdStr;
    int active = -1;
    std::vector<int> cursors(2, 0);
    std::wstring errorMsg;
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        // 对话框不清除整个屏幕，改为绘制对话框背景覆盖，以减少闪烁
        DrawShadowRect(240, 80, 840, 420, 12, RGB(255, 255, 255));
        settextstyle(26, 0, L"微软雅黑");
        outtextxy(400, 60, L"借书登记");
        settextstyle(18, 0, L"微软雅黑");
        outtextxy(300, 140, L"图书ID:");
        DrawInputBox(420, 140, 200, 36, bookIdStr, active == 0, cursors[0]);
        outtextxy(300, 200, L"读者ID:");
        DrawInputBox(420, 200, 200, 36, readerIdStr, active == 1, cursors[1]);
        if (!errorMsg.empty()) {
            settextcolor(RED);
            outtextxy(420, 280, errorMsg.c_str());
        }
        DrawButton(420, 340, 100, 40, L"确定");
        DrawButton(540, 340, 100, 40, L"取消");
        EndBatchDraw();

        m = getmessage(EX_CHAR | EX_KEY | EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            active = PtInRect(m.x, m.y, 420, 140, 200, 36) ? 0 :
                (PtInRect(m.x, m.y, 420, 200, 200, 36) ? 1 : -1);
            if (PtInRect(m.x, m.y, 420, 340, 100, 40)) {
                if (bookIdStr.empty() || readerIdStr.empty()) {
                    errorMsg = L"ID不能为空！";
                    continue;
                }
                int bid = std::stoi(ws2s(bookIdStr));
                int rid = std::stoi(ws2s(readerIdStr));

                auto itBook = std::find_if(g_books.begin(), g_books.end(),
                    [bid](const Book& b) { return b.id == bid; });
                auto itReader = std::find_if(g_readers.begin(), g_readers.end(),
                    [rid](const Reader& r) { return r.id == rid; });

                if (itBook == g_books.end() || itReader == g_readers.end()) {
                    errorMsg = L"图书或读者不存在！";
                    continue;
                }
                if (itBook->stock <= itBook->borrowed) {
                    errorMsg = L"该书已全部借出！";
                    continue;
                }

                BorrowRecord br;
                br.bookId = bid;
                br.bookTitle = itBook->title;
                br.readerId = rid;
                br.readerName = itReader->name;
                time_t now = std::time(nullptr);
                br.borrowDate = now;
                br.dueDate = now + 14 * 24 * 3600;
                br.returnDate = 0;

                g_borrows.push_back(br);
                itBook->borrowed++;
                SaveBorrows();
                SaveBooks();
                return;
            }
            if (PtInRect(m.x, m.y, 540, 340, 100, 40)) return;
        }
        else if (m.message == WM_CHAR && active >= 0) {
            wchar_t ch = m.ch;
            auto& buf = (active == 0) ? bookIdStr : readerIdStr;
            auto& cp = cursors[active];
            if (ch == '\b') { if (!buf.empty()) { buf.pop_back(); if (cp) --cp; } }
            else if (ch >= '0' && ch <= '9') { buf += ch; ++cp; }
        }
    }
}

// ---------- 还书操作 ----------
static void ReturnBook() {
    if (selectedRow < 0 || selectedRow >= (int)g_borrows.size()) return;
    BorrowRecord& br = g_borrows[selectedRow];
    if (br.returnDate != 0) return;

    auto it = std::find_if(g_books.begin(), g_books.end(),
        [&](const Book& b) { return b.id == br.bookId; });
    if (it != g_books.end() && it->borrowed > 0) it->borrowed--;

    br.returnDate = std::time(nullptr);
    SaveBorrows();
    SaveBooks();
    selectedRow = -1;
}

// ---------- 主界面 ----------
void borrowUIMain() {
    initgraph(WIN_W, WIN_H);
    SetWindowTextW(GetHWnd(), L"借阅管理 - 图书馆系统");
    ReloadBorrows();
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();
        DrawButton(20, 20, 100, 40, L"← 返回");
        DrawButton(140, 20, 100, 40, L"借书");
        DrawButton(260, 20, 100, 40, L"还书",
            selectedRow != -1 && selectedRow < (int)g_borrows.size() && g_borrows[selectedRow].returnDate == 0);
        DrawBorrowTable(30, 90);
        DrawPagination(30, WIN_H - 50, page, totalPage);
        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x, y = m.y;
            if (PtInRect(x, y, 20, 20, 100, 40)) return;
            if (PtInRect(x, y, 140, 20, 100, 40)) { BorrowDialog(); ReloadBorrows(); }
            if (PtInRect(x, y, 260, 20, 100, 40)) { ReturnBook(); ReloadBorrows(); }
            if (x >= 30 && x <= 900 && y >= 90 + ROW_H && y <= 90 + ROW_H * (PAGE_SIZE + 1)) {
                int row = (y - 90) / ROW_H - 1;
                int idx = page * PAGE_SIZE + row;
                if (idx >= 0 && idx < (int)g_borrows.size()) selectedRow = idx;
                else selectedRow = -1;
            }
            int pageBtnX = 30 + 150, pageBtnY = WIN_H - 58;
            if (PtInRect(x, y, pageBtnX, pageBtnY, 80, 30) && page > 0) --page;
            if (PtInRect(x, y, pageBtnX + 90, pageBtnY, 80, 30) && (page + 1) < totalPage) ++page;
        }
    }
}