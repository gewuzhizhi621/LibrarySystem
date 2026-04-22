#include "stat_ui.h"
#include "ui_utils.h"
#include "borrow_record.h"
#include "book_ui.h"          // 使用 Book、g_books、LoadBooks
#include "reader_ui.h"        // 使用 Reader、g_readers（虽未直接用，但保留以备后用）
#include <chrono>
#include <map>
#include <algorithm>

// 外部依赖（定义在 borrow_ui.cpp 中）
extern std::vector<BorrowRecord> g_borrows;
extern void ReloadBorrows();

// ---------- 借阅排行榜界面 ----------
static void ShowRanking() {
    LoadBooks();
    ReloadBorrows();

    std::map<int, int> borrowCount;
    std::map<int, std::string> bookTitleMap;
    for (const auto& b : g_books) bookTitleMap[b.id] = b.title;
    for (const auto& br : g_borrows) borrowCount[br.bookId]++;

    std::vector<std::pair<int, int>> rankVec(borrowCount.begin(), borrowCount.end());
    std::sort(rankVec.begin(), rankVec.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    int page = 0;
    int totalPage = (rankVec.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    if (totalPage == 0) totalPage = 1;
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();
        DrawButton(20, 20, 100, 40, L"← 返回");
        settextstyle(28, 0, L"微软雅黑");
        settextcolor(CLR_PRIMARY);
        outtextxy(400, 30, L"📈 图书借阅排行榜");

        std::vector<std::wstring> headers = { L"排名", L"图书ID", L"书名", L"借阅次数" };
        std::vector<int> widths = { 80, 100, 350, 120 };
        DrawTableHeader(30, 100, headers, widths);

        int start = page * PAGE_SIZE;
        for (int i = 0; i < PAGE_SIZE && start + i < (int)rankVec.size(); ++i) {
            int bookId = rankVec[start + i].first;
            int count = rankVec[start + i].second;
            int y = 100 + ROW_H * (i + 1);
            int curX = 30;
            for (size_t j = 0; j < widths.size(); ++j) {
                DrawRoundedRect(curX, y, curX + widths[j], y + ROW_H, 0, WHITE, CLR_TEXT_LIGHT);
                curX += widths[j];
            }
            setbkmode(TRANSPARENT);
            settextcolor(CLR_TEXT_DARK);
            settextstyle(18, 0, L"微软雅黑");
            curX = 30;
            outtextxy(curX + 10, y + 8, std::to_wstring(start + i + 1).c_str()); curX += widths[0];
            outtextxy(curX + 10, y + 8, std::to_wstring(bookId).c_str()); curX += widths[1];
            outtextxy(curX + 10, y + 8, s2ws(bookTitleMap[bookId]).c_str()); curX += widths[2];
            outtextxy(curX + 10, y + 8, std::to_wstring(count).c_str());
        }

        DrawPagination(30, WIN_H - 50, page, totalPage);
        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x, y = m.y;
            if (PtInRect(x, y, 20, 20, 100, 40)) return;
            int pageBtnX = 30 + 150, pageBtnY = WIN_H - 58;
            if (PtInRect(x, y, pageBtnX, pageBtnY, 80, 30) && page > 0) --page;
            if (PtInRect(x, y, pageBtnX + 90, pageBtnY, 80, 30) && (page + 1) < totalPage) ++page;
        }
    }
}

// ---------- 到期未还界面 ----------
static void ShowOverdue() {
    ReloadBorrows();
    time_t now = std::time(nullptr);
    std::vector<BorrowRecord> overdue;
    for (const auto& br : g_borrows)
        if (br.returnDate == 0 && br.dueDate < now)
            overdue.push_back(br);

    int page = 0;
    int totalPage = (overdue.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    if (totalPage == 0) totalPage = 1;
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();

        DrawButton(20, 20, 100, 40, L"← 返回");
        settextstyle(28, 0, L"微软雅黑");
        settextcolor(CLR_ACCENT);
        outtextxy(380, 30, L"⚠️ 逾期未还图书");

        std::vector<std::wstring> headers = { L"图书ID", L"书名", L"读者", L"应还日期", L"逾期天数" };
        std::vector<int> widths = { 80, 280, 180, 150, 100 };
        DrawTableHeader(30, 100, headers, widths);

        int start = page * PAGE_SIZE;
        for (int i = 0; i < PAGE_SIZE && start + i < (int)overdue.size(); ++i) {
            const BorrowRecord& br = overdue[start + i];
            int y = 100 + ROW_H * (i + 1);
            int curX = 30;
            for (size_t j = 0; j < widths.size(); ++j) {
                DrawRoundedRect(curX, y, curX + widths[j], y + ROW_H, 0, WHITE, CLR_TEXT_LIGHT);
                curX += widths[j];
            }
            setbkmode(TRANSPARENT);
            settextcolor(CLR_TEXT_DARK);
            settextstyle(18, 0, L"微软雅黑");
            curX = 30;
            outtextxy(curX + 5, y + 8, std::to_wstring(br.bookId).c_str()); curX += widths[0];
            outtextxy(curX + 5, y + 8, s2ws(br.bookTitle).c_str()); curX += widths[1];
            outtextxy(curX + 5, y + 8, s2ws(br.readerName).c_str()); curX += widths[2];
            outtextxy(curX + 5, y + 8, timeToWstring(br.dueDate).c_str()); curX += widths[3];
            int days = (now - br.dueDate) / (24 * 3600);
            outtextxy(curX + 5, y + 8, (std::to_wstring(days) + L" 天").c_str());
        }

        DrawPagination(30, WIN_H - 50, page, totalPage);
        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x, y = m.y;
            if (PtInRect(x, y, 20, 20, 100, 40)) return;
            int pageBtnX = 30 + 150, pageBtnY = WIN_H - 58;
            if (PtInRect(x, y, pageBtnX, pageBtnY, 80, 30) && page > 0) --page;
            if (PtInRect(x, y, pageBtnX + 90, pageBtnY, 80, 30) && (page + 1) < totalPage) ++page;
        }
    }
}

// ---------- 统计分析主菜单 ----------
void statUIMain() {
    EnsureGraphInitialized();
    SetWindowTextW(GetHWnd(), L"统计分析 - 图书馆系统");
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();

        DrawButton(20, 20, 100, 40, L"← 返回");
        settextstyle(32, 0, L"微软雅黑");
        settextcolor(CLR_PRIMARY);
        outtextxy(380, 100, L"统计分析功能");

        DrawButton(350, 220, 300, 60, L"借阅排行榜");
        DrawButton(350, 320, 300, 60, L"逾期未还清单");

        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x, y = m.y;
            if (PtInRect(x, y, 20, 20, 100, 40)) return;
            if (PtInRect(x, y, 350, 220, 300, 60)) { ShowRanking(); }
            if (PtInRect(x, y, 350, 320, 300, 60)) { ShowOverdue(); }
        }
    }
}