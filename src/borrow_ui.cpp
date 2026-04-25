#include "borrow_ui.h"
#include "book_ui.h"
#include "log_ui.h"
#include "reader_ui.h"
#include "ui_utils.h"

#include <algorithm>
#include <ctime>
#include <string>
#include <vector>

std::vector<BorrowRecord> g_borrows;

static int page = 0;
static int totalPage = 1;
static int selectedRow = -1;
static std::wstring searchKey;
static bool showOnlyActive = false;
static bool showOnlyOverdue = false;
static std::vector<int> displayRows;

void ReloadBorrows() {
    LoadCSV("data/borrow_records.csv", g_borrows, true);
}

void SaveBorrows() {
    SaveCSV("data/borrow_records.csv", g_borrows, "bookId,bookTitle,readerId,readerName,borrowDate,dueDate,returnDate");
}

static int ActiveBorrowCount(int readerId) {
    int count = 0;
    for (const auto& br : g_borrows) {
        if (br.readerId == readerId && br.returnDate == 0) {
            ++count;
        }
    }
    return count;
}

static bool HasOverdue(int readerId) {
    time_t now = time(nullptr);
    for (const auto& br : g_borrows) {
        if (br.readerId == readerId && br.returnDate == 0 && br.dueDate != 0 && br.dueDate < now) {
            return true;
        }
    }
    return false;
}

static bool MatchBorrow(const BorrowRecord& br, const std::string& key) {
    bool overdue = br.returnDate == 0 && br.dueDate != 0 && br.dueDate < time(nullptr);
    if (showOnlyActive && br.returnDate != 0) {
        return false;
    }
    if (showOnlyOverdue && !overdue) {
        return false;
    }
    if (key.empty()) {
        return true;
    }
    return br.bookTitle.find(key) != std::string::npos ||
        br.readerName.find(key) != std::string::npos ||
        std::to_string(br.bookId).find(key) != std::string::npos ||
        std::to_string(br.readerId).find(key) != std::string::npos;
}

static void RefreshRows() {
    displayRows.clear();
    std::string key = ws2s(searchKey);
    for (int i = 0; i < static_cast<int>(g_borrows.size()); ++i) {
        if (MatchBorrow(g_borrows[i], key)) {
            displayRows.push_back(i);
        }
    }
    totalPage = (static_cast<int>(displayRows.size()) + PAGE_SIZE - 1) / PAGE_SIZE;
    if (totalPage <= 0) {
        totalPage = 1;
    }
    if (page >= totalPage) {
        page = totalPage - 1;
    }
    if (page < 0) {
        page = 0;
    }
}

static void DrawBorrowTable(int x, int y, const std::vector<int>& rows, bool selectable) {
    std::vector<std::wstring> headers = { L"图书ID", L"书名", L"读者ID", L"读者", L"借书日", L"应还日", L"归还日", L"状态" };
    std::vector<int> widths = { 75, 240, 75, 110, 120, 120, 120, 120 };
    DrawTableHeader(x, y, headers, widths);

    int start = page * PAGE_SIZE;
    for (int i = 0; i < PAGE_SIZE && start + i < static_cast<int>(rows.size()); ++i) {
        int realIndex = rows[start + i];
        const BorrowRecord& br = g_borrows[realIndex];
        bool overdue = br.returnDate == 0 && br.dueDate != 0 && br.dueDate < time(nullptr);
        COLORREF bg = selectable && selectedRow == realIndex ? RGB(230, 241, 255) : (overdue ? RGB(255, 237, 237) : WHITE);
        COLORREF color = overdue ? RGB(176, 38, 38) : CLR_TEXT_DARK;
        std::wstring status = br.returnDate != 0 ? L"已归还" : (overdue ? L"已逾期" : L"借阅中");
        int rowY = y + ROW_H * (i + 1);
        int cur = x;

        DrawCell(cur, rowY, widths[0], std::to_wstring(br.bookId), bg, color); cur += widths[0];
        DrawCell(cur, rowY, widths[1], s2ws(br.bookTitle), bg, color); cur += widths[1];
        DrawCell(cur, rowY, widths[2], std::to_wstring(br.readerId), bg, color); cur += widths[2];
        DrawCell(cur, rowY, widths[3], s2ws(br.readerName), bg, color); cur += widths[3];
        DrawCell(cur, rowY, widths[4], timeToWstring(br.borrowDate), bg, color); cur += widths[4];
        DrawCell(cur, rowY, widths[5], timeToWstring(br.dueDate), bg, color); cur += widths[5];
        DrawCell(cur, rowY, widths[6], br.returnDate == 0 ? L"未归还" : timeToWstring(br.returnDate), bg, color); cur += widths[6];
        DrawCell(cur, rowY, widths[7], status, bg, color);
    }
}

static void BorrowBook() {
    ReloadBorrows();
    LoadBooks();
    LoadReaders();

    std::vector<FormField> fields = {
        { L"图书ID", L"" },
        { L"读者ID", L"" }
    };
    if (!ShowFormDialog(L"借书", fields, 420)) {
        return;
    }

    int bookId = 0;
    int readerId = 0;
    try {
        bookId = std::stoi(fields[0].value);
        readerId = std::stoi(fields[1].value);
    } catch (...) {
        MessageBoxW(GetHWnd(), L"图书ID 和 读者ID 必须为整数。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    auto bookIt = std::find_if(g_books.begin(), g_books.end(), [&](const Book& b) { return b.id == bookId; });
    auto readerIt = std::find_if(g_readers.begin(), g_readers.end(), [&](const Reader& r) { return r.id == readerId; });

    if (bookIt == g_books.end()) {
        MessageBoxW(GetHWnd(), L"图书不存在。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    if (readerIt == g_readers.end()) {
        MessageBoxW(GetHWnd(), L"读者不存在。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    if (readerIt->status == "disabled") {
        MessageBoxW(GetHWnd(), L"该读者账号已停用。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    if (bookIt->borrowed >= bookIt->stock) {
        MessageBoxW(GetHWnd(), L"图书库存不足。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    if (HasOverdue(readerId)) {
        MessageBoxW(GetHWnd(), L"该读者存在逾期图书，暂不能借书。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    if (ActiveBorrowCount(readerId) >= 5) {
        MessageBoxW(GetHWnd(), L"该读者当前借阅已达到 5 本上限。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }

    BorrowRecord br;
    br.bookId = bookIt->id;
    br.bookTitle = bookIt->title;
    br.readerId = readerIt->id;
    br.readerName = readerIt->name;
    br.borrowDate = time(nullptr);
    br.dueDate = br.borrowDate + 14 * 24 * 3600;
    br.returnDate = 0;

    g_borrows.push_back(br);
    ++bookIt->borrowed;
    ++bookIt->totalBorrowed;
    SaveBorrows();
    SaveBooks();
    AddLog("admin", "admin", "借书", br.bookTitle + " -> " + br.readerName);
    MessageBoxW(GetHWnd(), L"借书成功。", L"提示", MB_OK | MB_ICONINFORMATION);
}

static void ReturnBook() {
    if (selectedRow < 0 || selectedRow >= static_cast<int>(g_borrows.size())) {
        MessageBoxW(GetHWnd(), L"请先选择借阅记录。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    BorrowRecord& br = g_borrows[selectedRow];
    if (br.returnDate != 0) {
        MessageBoxW(GetHWnd(), L"该记录已经归还。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    LoadBooks();
    auto bookIt = std::find_if(g_books.begin(), g_books.end(), [&](const Book& b) { return b.id == br.bookId; });
    if (bookIt != g_books.end() && bookIt->borrowed > 0) {
        --bookIt->borrowed;
    }
    br.returnDate = time(nullptr);
    SaveBorrows();
    SaveBooks();
    AddLog("admin", "admin", "还书", br.bookTitle + " <- " + br.readerName);
    MessageBoxW(GetHWnd(), L"还书成功。", L"提示", MB_OK | MB_ICONINFORMATION);
}

static void RenewBook() {
    if (selectedRow < 0 || selectedRow >= static_cast<int>(g_borrows.size())) {
        MessageBoxW(GetHWnd(), L"请先选择借阅记录。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    BorrowRecord& br = g_borrows[selectedRow];
    if (br.returnDate != 0) {
        MessageBoxW(GetHWnd(), L"已归还记录不能续借。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    br.dueDate += 7 * 24 * 3600;
    SaveBorrows();
    AddLog("admin", "admin", "续借", br.bookTitle + " -> " + br.readerName);
    MessageBoxW(GetHWnd(), L"续借成功，应还日期已延长 7 天。", L"提示", MB_OK | MB_ICONINFORMATION);
}

static void SearchRecords() {
    AskText(L"搜索借阅记录", L"请输入关键词", searchKey, searchKey);
    page = 0;
    selectedRow = -1;
    RefreshRows();
}

void borrowUIMain() {
    EnsureGraphInitialized();
    EnsureBaseDataFiles();
    SetWindowTextW(GetHWnd(), L"DUT Library System - 借阅管理");
    ReloadBorrows();
    RefreshRows();
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();
        DrawHeaderPanel(L"借阅管理");

        int by = 86;
        DrawBackButton(20, by);
        DrawButton(150, by, 105, 40, L"借书");
        DrawButton(270, by, 105, 40, L"还书", selectedRow != -1);
        DrawButton(390, by, 105, 40, L"续借", selectedRow != -1, CLR_SUCCESS);
        DrawButton(660, by, 95, 40, L"搜索");
        DrawButton(770, by, 120, 40, showOnlyActive ? L"全部记录" : L"只看未还");
        DrawButton(905, by, 120, 40, showOnlyOverdue ? L"全部记录" : L"只看逾期", true, CLR_WARNING);
        DrawButton(1035, by, 50, 40, L"重置", true, CLR_SUCCESS);

        setbkmode(TRANSPARENT);
        settextcolor(CLR_DANGER);
        settextstyle_w(17, 0, L"微软雅黑");
        outtextxy_w(22, 140, L"红色记录表示逾期；支持：借书、还书、续借、搜索、只看未还、只看逾期");

        DrawBorrowTable(20, 170, displayRows, true);
        DrawPagination(20, WIN_H - 38, page, totalPage);
        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x;
            int y = m.y;
            int by = 86;
            if (PtInRect(x, y, 20, by, 105, 40)) {
                return;
            }
            if (PtInRect(x, y, 150, by, 105, 40)) {
                BorrowBook();
                ReloadBorrows();
                RefreshRows();
            }
            if (PtInRect(x, y, 270, by, 105, 40)) {
                ReturnBook();
                ReloadBorrows();
                RefreshRows();
            }
            if (PtInRect(x, y, 390, by, 105, 40)) {
                RenewBook();
                ReloadBorrows();
                RefreshRows();
            }
            if (PtInRect(x, y, 660, by, 95, 40)) {
                SearchRecords();
            }
            if (PtInRect(x, y, 770, by, 120, 40)) {
                showOnlyActive = !showOnlyActive;
                showOnlyOverdue = false;
                page = 0;
                selectedRow = -1;
                RefreshRows();
            }
            if (PtInRect(x, y, 905, by, 120, 40)) {
                showOnlyOverdue = !showOnlyOverdue;
                showOnlyActive = false;
                page = 0;
                selectedRow = -1;
                RefreshRows();
            }
            if (PtInRect(x, y, 1035, by, 50, 40)) {
                searchKey.clear();
                showOnlyActive = false;
                showOnlyOverdue = false;
                selectedRow = -1;
                page = 0;
                RefreshRows();
            }
            if (x >= 20 && x <= 1010 && y >= 170 + ROW_H && y <= 170 + ROW_H * (PAGE_SIZE + 1)) {
                int row = (y - 170) / ROW_H - 1;
                int di = page * PAGE_SIZE + row;
                selectedRow = (di >= 0 && di < static_cast<int>(displayRows.size())) ? displayRows[di] : -1;
            }
            int prevX = 0;
            int nextX = 0;
            GetPaginationButtonPos(prevX, nextX);
            if (PtInRect(x, y, prevX, WIN_H - 46, 90, 34) && page > 0) {
                --page;
            }
            if (PtInRect(x, y, nextX, WIN_H - 46, 90, 34) && page + 1 < totalPage) {
                ++page;
            }
        }
    }
}

void borrowMyRecordsUIMain(int readerId) {
    EnsureGraphInitialized();
    SetWindowTextW(GetHWnd(), L"DUT Library System - 我的借阅");
    ReloadBorrows();
    std::vector<int> myRows;
    for (int i = 0; i < static_cast<int>(g_borrows.size()); ++i) {
        if (g_borrows[i].readerId == readerId) {
            myRows.push_back(i);
        }
    }

    int myPage = 0;
    int myTotal = (static_cast<int>(myRows.size()) + PAGE_SIZE - 1) / PAGE_SIZE;
    if (myTotal <= 0) {
        myTotal = 1;
    }

    ExMessage m;
    while (true) {
        page = myPage;
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();
        DrawHeaderPanel(L"我的借阅记录");
        DrawBackButton(20, 86);
        setbkmode(TRANSPARENT);
        settextcolor(CLR_TEXT_LIGHT);
        settextstyle_w(17, 0, L"微软雅黑");
        outtextxy_w(22, 140, L"红色记录表示逾期；当前页面每页显示 10 条借阅记录");
        DrawBorrowTable(20, 170, myRows, false);
        DrawPagination(20, WIN_H - 38, myPage, myTotal);
        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            if (PtInRect(m.x, m.y, 20, 86, 105, 40)) {
                return;
            }
            int prevX = 0;
            int nextX = 0;
            GetPaginationButtonPos(prevX, nextX);
            if (PtInRect(m.x, m.y, prevX, WIN_H - 46, 90, 34) && myPage > 0) {
                --myPage;
            }
            if (PtInRect(m.x, m.y, nextX, WIN_H - 46, 90, 34) && myPage + 1 < myTotal) {
                ++myPage;
            }
        }
    }
}
