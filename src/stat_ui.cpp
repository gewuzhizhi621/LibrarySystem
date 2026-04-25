#include "stat_ui.h"
#include "book_ui.h"
#include "borrow_ui.h"
#include "reader_ui.h"
#include "ui_utils.h"

#include <algorithm>
#include <ctime>
#include <map>
#include <vector>

struct RankItem {
    int bookId = 0;
    std::wstring title;
    int count = 0;
};

static int CountBorrowing() {
    int count = 0;

    for (const auto& br : g_borrows) {
        if (br.returnDate == 0) {
            count++;
        }
    }

    return count;
}

static int CountOverdue() {
    int count = 0;
    time_t now = time(nullptr);

    for (const auto& br : g_borrows) {
        if (br.returnDate == 0 && br.dueDate < now) {
            count++;
        }
    }

    return count;
}

static int CountTotalStock() {
    int total = 0;

    for (const auto& book : g_books) {
        total += book.stock;
    }

    return total;
}

static std::vector<RankItem> GetRanking() {
    std::vector<RankItem> rank;

    for (const auto& book : g_books) {
        if (book.totalBorrowed > 0) {
            RankItem item;
            item.bookId = book.id;
            item.title = s2ws(book.title);
            item.count = book.totalBorrowed;
            rank.push_back(item);
        }
    }

    std::sort(rank.begin(), rank.end(), [](const RankItem& a, const RankItem& b) {
        return a.count > b.count;
    });

    return rank;
}

static void DrawCard(
    int x,
    int y,
    int width,
    int height,
    const std::wstring& title,
    const std::wstring& value,
    COLORREF color
) {
    setfillcolor(WHITE);
    setlinecolor(RGB(220, 230, 245));
    solidroundrect(x, y, x + width, y + height, 16, 16);
    roundrect(x, y, x + width, y + height, 16, 16);

    setfillcolor(color);
    solidroundrect(x, y, x + 10, y + height, 10, 10);

    setbkmode(TRANSPARENT);
    settextcolor(CLR_TEXT_LIGHT);
    settextstyle_w(18, 0, L"微软雅黑");
    outtextxy_w(x + 28, y + 18, title);

    settextcolor(color);
    settextstyle_w(34, 0, L"微软雅黑");
    outtextxy_w(x + 28, y + 55, value);
}

static void DrawBarChart(int x, int y, int width, int height, const std::vector<RankItem>& rank) {
    setfillcolor(WHITE);
    setlinecolor(RGB(220, 230, 245));
    solidroundrect(x, y, x + width, y + height, 18, 18);
    roundrect(x, y, x + width, y + height, 18, 18);

    setbkmode(TRANSPARENT);
    settextcolor(CLR_PRIMARY);
    settextstyle_w(24, 0, L"微软雅黑");
    outtextxy_w(x + 25, y + 20, L"热门图书柱状图 TOP 5");

    if (rank.empty()) {
        settextcolor(CLR_TEXT_LIGHT);
        outtextxy_w(x + 25, y + 85, L"暂无借阅数据");
        return;
    }

    int topN = (std::min)(5, static_cast<int>(rank.size()));
    int maxCount = 1;

    for (const auto& item : rank) {
        maxCount = (std::max)(maxCount, item.count);
    }

    int baseX = x + 80;
    int baseY = y + height - 55;
    int chartHeight = height - 130;
    int barWidth = 55;
    int gap = 45;

    setlinecolor(RGB(180, 190, 205));
    line(baseX - 20, baseY, baseX + topN * (barWidth + gap), baseY);
    line(baseX - 20, baseY, baseX - 20, baseY - chartHeight);

    for (int i = 0; i < topN; ++i) {
        int barHeight = rank[i].count * chartHeight / maxCount;
        int bx = baseX + i * (barWidth + gap);
        int by = baseY - barHeight;

        setfillcolor(CLR_PRIMARY);
        solidroundrect(bx, by, bx + barWidth, baseY, 8, 8);

        std::wstring countText = std::to_wstring(rank[i].count);
        settextcolor(CLR_TEXT_DARK);
        settextstyle_w(16, 0, L"微软雅黑");
        outtextxy_w(bx + (barWidth - textwidth_w(countText)) / 2, by - 25, countText);

        std::wstring name = rank[i].title;

        if (name.size() > 5) {
            name = name.substr(0, 5) + L"...";
        }

        settextcolor(CLR_TEXT_LIGHT);
        settextstyle_w(14, 0, L"微软雅黑");
        outtextxy_w(bx + (barWidth - textwidth_w(name)) / 2, baseY + 10, name);
    }
}

static void DrawOverdueList(int x, int y, int width, int height) {
    setfillcolor(WHITE);
    setlinecolor(RGB(220, 230, 245));
    solidroundrect(x, y, x + width, y + height, 18, 18);
    roundrect(x, y, x + width, y + height, 18, 18);

    setbkmode(TRANSPARENT);
    settextcolor(CLR_DANGER);
    settextstyle_w(24, 0, L"微软雅黑");
    outtextxy_w(x + 25, y + 20, L"逾期清单");

    std::vector<BorrowRecord> overdueList;
    time_t now = time(nullptr);

    for (const auto& br : g_borrows) {
        if (br.returnDate == 0 && br.dueDate < now) {
            overdueList.push_back(br);
        }
    }

    if (overdueList.empty()) {
        settextcolor(CLR_TEXT_LIGHT);
        settextstyle_w(18, 0, L"微软雅黑");
        outtextxy_w(x + 25, y + 80, L"暂无逾期记录");
        return;
    }

    std::sort(overdueList.begin(), overdueList.end(), [](const BorrowRecord& a, const BorrowRecord& b) {
        return a.dueDate < b.dueDate;
    });

    int showCount = (std::min)(8, static_cast<int>(overdueList.size()));

    settextcolor(CLR_TEXT_DARK);
    settextstyle_w(15, 0, L"微软雅黑");

    for (int i = 0; i < showCount; ++i) {
        const auto& br = overdueList[i];
        std::wstring lineText = std::to_wstring(i + 1) + L". " + s2ws(br.readerName) + L" - " + s2ws(br.bookTitle) + L" / " + timeToWstring(br.dueDate);
        outtextxy_w(x + 25, y + 70 + i * 32, lineText);
    }
}

void statUIMain() {
    EnsureGraphInitialized();
    EnsureBaseDataFiles();
    SetWindowTextW(GetHWnd(), L"DUT Library System - 数据统计中心");

    ExMessage m;

    while (true) {
        LoadBooks();
        LoadReaders();
        ReloadBorrows();

        int totalBooks = static_cast<int>(g_books.size());
        int totalStock = CountTotalStock();
        int totalReaders = static_cast<int>(g_readers.size());
        int borrowing = CountBorrowing();
        int overdue = CountOverdue();
        auto rank = GetRanking();

        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();
        DrawTitle(L"数据统计中心");
        DrawBackButton(20, 86);
        DrawButton(980, 86, 90, 38, L"刷新", true, CLR_SUCCESS);

        DrawCard(40, 150, 185, 95, L"图书种类", std::to_wstring(totalBooks), CLR_PRIMARY);
        DrawCard(245, 150, 185, 95, L"库存总量", std::to_wstring(totalStock), RGB(90, 130, 220));
        DrawCard(450, 150, 185, 95, L"读者总数", std::to_wstring(totalReaders), CLR_SUCCESS);
        DrawCard(655, 150, 185, 95, L"当前借出", std::to_wstring(borrowing), CLR_WARNING);
        DrawCard(860, 150, 185, 95, L"逾期", std::to_wstring(overdue), CLR_DANGER);

        DrawBarChart(40, 275, 640, 355, rank);
        DrawOverdueList(710, 275, 340, 355);
        EndBatchDraw();

        m = getmessage(EX_MOUSE);

        if (m.message == WM_LBUTTONDOWN) {
            if (PtInRect(m.x, m.y, 20, 86, 105, 40)) {
                return;
            }
        }
    }
}
