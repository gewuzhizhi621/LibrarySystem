#include "book_ui.h"
#include "ui_utils.h"
#include <algorithm>
#include <chrono>



std::vector<Book> g_books;
static int page = 0, totalPage = 0;
static std::wstring searchKey;
static int selectedRow = -1;
static bool searchMode = false;

void LoadBooks() {
    LoadCSV("data/books.csv", g_books, true);
    totalPage = (g_books.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    if (totalPage == 0) totalPage = 1;
}

void SaveBooks() {
    SaveCSV("data/books.csv", g_books, "id,title,author,publisher,stock,borrowed");
}

static void DrawBookTable(int x0, int y0) {
    std::vector<std::wstring> headers = { L"ID", L"书名", L"作者", L"出版社", L"库存", L"借出" };
    std::vector<int> widths = { 60, 220, 150, 180, 80, 80 };
    DrawTableHeader(x0, y0, headers, widths);

    int start = page * PAGE_SIZE;
    for (int i = 0; i < PAGE_SIZE && start + i < (int)g_books.size(); ++i) {
        const Book& b = g_books[start + i];
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
        outtextxy(curX + 5, y + 8, std::to_wstring(b.id).c_str()); curX += widths[0];
        outtextxy(curX + 5, y + 8, s2ws(b.title).c_str()); curX += widths[1];
        outtextxy(curX + 5, y + 8, s2ws(b.author).c_str()); curX += widths[2];
        outtextxy(curX + 5, y + 8, s2ws(b.publisher).c_str()); curX += widths[3];
        outtextxy(curX + 5, y + 8, std::to_wstring(b.stock).c_str()); curX += widths[4];
        outtextxy(curX + 5, y + 8, std::to_wstring(b.borrowed).c_str());
    }
}

static void AddBookDialog() {
    std::wstring title, author, publisher, stock;
    std::vector<std::wstring*> fields = { &title, &author, &publisher, &stock };
    const wchar_t* labels[] = { L"书名", L"作者", L"出版社", L"库存数量" };
    int activeField = -1;
    std::vector<int> cursors(4, 0);
    std::wstring errorMsg;

    ExMessage m;
    while (true) {
        BeginBatchDraw();
        // 不在对话框中清屏，仅绘制一个对话框背景覆盖当前画面，避免闪烁
        DrawShadowRect(240, 80, 840, 520, 12, RGB(255, 255, 255));
        settextstyle(26, 0, L"微软雅黑");
        settextcolor(CLR_TEXT_DARK);
        outtextxy(400, 60, L"添加新书");
        settextstyle(18, 0, L"微软雅黑");
        for (int i = 0; i < 4; ++i) {
            outtextxy(300, 140 + i * 60, labels[i]);
            DrawInputBox(420, 140 + i * 60, 280, 36, *fields[i], activeField == i, cursors[i]);
        }
        if (!errorMsg.empty()) {
            settextcolor(RED);
            outtextxy(420, 400, errorMsg.c_str());
        }
        DrawButton(420, 460, 100, 40, L"确定");
        DrawButton(540, 460, 100, 40, L"取消");
        EndBatchDraw();

        m = getmessage(EX_CHAR | EX_KEY | EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            activeField = -1;
            for (int i = 0; i < 4; ++i)
                if (PtInRect(m.x, m.y, 420, 140 + i * 60, 280, 36)) { activeField = i; break; }
            if (PtInRect(m.x, m.y, 420, 460, 100, 40)) {
                if (title.empty() || author.empty() || publisher.empty() || stock.empty()) {
                    errorMsg = L"所有字段均不能为空！";
                    continue;
                }
                Book b;
                b.id = g_books.empty() ? 1 : g_books.back().id + 1;
                b.title = ws2s(title);
                b.author = ws2s(author);
                b.publisher = ws2s(publisher);
                b.stock = std::stoi(ws2s(stock));
                b.borrowed = 0;
                g_books.push_back(b);
                SaveBooks();
                return;
            }
            if (PtInRect(m.x, m.y, 540, 460, 100, 40)) return;
        }
        else if (m.message == WM_CHAR && activeField >= 0) {
            wchar_t ch = m.ch;
            auto& buf = *fields[activeField];
            auto& cp = cursors[activeField];
            if (ch == '\b') { if (!buf.empty()) { buf.pop_back(); if (cp) --cp; } }
            else if (ch >= 32) { buf += ch; ++cp; }
        }
    }
}

static void DeleteSelected() {
    if (selectedRow >= 0 && selectedRow < (int)g_books.size()) {
        g_books.erase(g_books.begin() + selectedRow);
        SaveBooks();
        selectedRow = -1;
        if (page >= totalPage && page > 0) --page;
    }
}

static void SearchBooks(const std::wstring& key) {
    if (key.empty()) {
        LoadBooks();
        searchMode = false;
    }
    else {
        std::string k = ws2s(key);
        std::vector<Book> filtered;
        for (const auto& b : g_books) {
            if (b.title.find(k) != std::string::npos ||
                b.author.find(k) != std::string::npos ||
                b.publisher.find(k) != std::string::npos)
                filtered.push_back(b);
        }
        g_books = filtered;
        searchMode = true;
    }
    page = 0;
    totalPage = (g_books.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    if (totalPage == 0) totalPage = 1;
    selectedRow = -1;
}

void bookUIMain() {
    initgraph(WIN_W, WIN_H);
    SetWindowTextW(GetHWnd(), L"图书管理 - 图书馆系统");
    LoadBooks();
    bool searchActive = false;
    int cursorPos = 0;
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();
        DrawButton(20, 20, 100, 40, L"← 返回");
        DrawButton(140, 20, 100, 40, L"添加图书");
        DrawButton(260, 20, 100, 40, L"删除选中", selectedRow != -1);
        settextstyle(18, 0, L"微软雅黑");
        settextcolor(CLR_TEXT_DARK);
        outtextxy(400, 32, L"搜索:");
        DrawInputBox(460, 20, 220, 36, searchKey, searchActive, cursorPos);
        DrawButton(690, 20, 80, 36, L"搜索");
        DrawButton(780, 20, 80, 36, L"重置");

        DrawBookTable(30, 90);
        DrawPagination(30, WIN_H - 50, page, totalPage);
        EndBatchDraw();

        m = getmessage(EX_CHAR | EX_KEY | EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x, y = m.y;
            if (PtInRect(x, y, 20, 20, 100, 40)) return;
            if (PtInRect(x, y, 140, 20, 100, 40)) { AddBookDialog(); LoadBooks(); }
            if (PtInRect(x, y, 260, 20, 100, 40) && selectedRow != -1) { DeleteSelected(); LoadBooks(); }
            if (PtInRect(x, y, 460, 20, 220, 36)) { searchActive = true; cursorPos = (int)searchKey.length(); }
            else searchActive = false;
            if (PtInRect(x, y, 690, 20, 80, 36)) { SearchBooks(searchKey); }
            if (PtInRect(x, y, 780, 20, 80, 36)) { searchKey.clear(); LoadBooks(); }
            // 表格点击选中
            if (x >= 30 && x <= 800 && y >= 90 + ROW_H && y <= 90 + ROW_H * (PAGE_SIZE + 1)) {
                int row = (y - 90) / ROW_H - 1;
                int idx = page * PAGE_SIZE + row;
                if (idx >= 0 && idx < (int)g_books.size()) selectedRow = idx;
                else selectedRow = -1;
            }
            // 分页
            int pageBtnX = 30 + 150, pageBtnY = WIN_H - 58;
            if (PtInRect(x, y, pageBtnX, pageBtnY, 80, 30) && page > 0) --page;
            if (PtInRect(x, y, pageBtnX + 90, pageBtnY, 80, 30) && (page + 1) < totalPage) ++page;
        }
        else if (m.message == WM_CHAR && searchActive) {
            wchar_t ch = m.ch;
            if (ch == '\b') { if (!searchKey.empty()) { searchKey.pop_back(); if (cursorPos) --cursorPos; } }
            else if (ch >= 32) { searchKey += ch; ++cursorPos; }
        }
    }
}