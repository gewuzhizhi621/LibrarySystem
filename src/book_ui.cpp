#include "book_ui.h"
#include "ui_utils.h"
#include "log_ui.h"

#include <algorithm>
#include <string>
#include <vector>

std::vector<Book> g_books;

static int page = 0;
static int totalPage = 1;
static int selectedRow = -1;
static std::wstring searchKey;
static std::wstring categoryFilter = L"全部";
static int sortMode = 0;
static std::vector<int> displayRows;

void LoadBooks() {
    LoadCSV("data/books.csv", g_books, true);
}

void SaveBooks() {
    SaveCSV("data/books.csv", g_books, "id,title,author,publisher,category,stock,borrowed,totalBorrowed");
}

static int NextBookId() {
    int maxId = 0;
    for (const auto& b : g_books) {
        maxId = (std::max)(maxId, b.id);
    }
    return maxId + 1;
}

static bool ContainsText(const std::string& text, const std::string& key) {
    return text.find(key) != std::string::npos;
}

static bool MatchBook(const Book& b, const std::string& key, const std::string& category) {
    bool keyMatch = key.empty() ||
        ContainsText(b.title, key) ||
        ContainsText(b.author, key) ||
        ContainsText(b.publisher, key) ||
        ContainsText(b.category, key) ||
        ContainsText(std::to_string(b.id), key);
    bool categoryMatch = category.empty() || category == "全部" || b.category == category;
    return keyMatch && categoryMatch;
}

static void RefreshRows() {
    displayRows.clear();
    std::string key = ws2s(searchKey);
    std::string category = ws2s(categoryFilter);

    for (int i = 0; i < static_cast<int>(g_books.size()); ++i) {
        if (MatchBook(g_books[i], key, category)) {
            displayRows.push_back(i);
        }
    }

    std::sort(displayRows.begin(), displayRows.end(), [](int a, int b) {
        if (sortMode == 1) {
            return g_books[a].title < g_books[b].title;
        }
        if (sortMode == 2) {
            return (g_books[a].stock - g_books[a].borrowed) > (g_books[b].stock - g_books[b].borrowed);
        }
        if (sortMode == 3) {
            return g_books[a].totalBorrowed > g_books[b].totalBorrowed;
        }
        return g_books[a].id < g_books[b].id;
    });

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

static void DrawBookTable(int x, int y) {
    std::vector<std::wstring> headers = { L"ID", L"书名", L"作者", L"出版社", L"分类", L"库存", L"借出", L"总借阅" };
    std::vector<int> widths = { 55, 220, 150, 220, 120, 75, 75, 90 };
    DrawTableHeader(x, y, headers, widths);

    int start = page * PAGE_SIZE;
    for (int i = 0; i < PAGE_SIZE && start + i < static_cast<int>(displayRows.size()); ++i) {
        int idx = displayRows[start + i];
        const Book& b = g_books[idx];
        int rowY = y + ROW_H * (i + 1);
        int cur = x;
        int available = b.stock - b.borrowed;
        COLORREF bg = (selectedRow == idx) ? RGB(230, 241, 255) : WHITE;
        COLORREF color = available <= 1 ? CLR_DANGER : CLR_TEXT_DARK;

        DrawCell(cur, rowY, widths[0], std::to_wstring(b.id), bg, color); cur += widths[0];
        DrawCell(cur, rowY, widths[1], s2ws(b.title), bg, color); cur += widths[1];
        DrawCell(cur, rowY, widths[2], s2ws(b.author), bg, color); cur += widths[2];
        DrawCell(cur, rowY, widths[3], s2ws(b.publisher), bg, color); cur += widths[3];
        DrawCell(cur, rowY, widths[4], s2ws(b.category), bg, color); cur += widths[4];
        DrawCell(cur, rowY, widths[5], std::to_wstring(b.stock), bg, color); cur += widths[5];
        DrawCell(cur, rowY, widths[6], std::to_wstring(b.borrowed), bg, color); cur += widths[6];
        DrawCell(cur, rowY, widths[7], std::to_wstring(b.totalBorrowed), bg, color);
    }
}

static bool ShowBookForm(const std::wstring& title, Book& book, bool isEdit) {
    std::vector<FormField> fields = {
        { L"书名", s2ws(book.title) },
        { L"作者", s2ws(book.author) },
        { L"出版社", s2ws(book.publisher) },
        { L"分类", s2ws(book.category.empty() ? "计算机" : book.category) },
        { L"库存", std::to_wstring(book.stock <= 0 ? 1 : book.stock) }
    };

    if (!ShowFormDialog(title, fields, 500)) {
        return false;
    }

    for (const auto& f : fields) {
        if (f.value.empty()) {
            MessageBoxW(GetHWnd(), L"请完整填写所有信息。", L"错误", MB_OK | MB_ICONERROR);
            return false;
        }
    }

    int stock = 0;
    try {
        stock = std::stoi(fields[4].value);
    } catch (...) {
        MessageBoxW(GetHWnd(), L"库存必须是整数。", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }

    if (isEdit && stock < book.borrowed) {
        MessageBoxW(GetHWnd(), L"库存不能小于当前借出数量。", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }

    book.title = ws2s(fields[0].value);
    book.author = ws2s(fields[1].value);
    book.publisher = ws2s(fields[2].value);
    book.category = ws2s(fields[3].value);
    book.stock = stock;
    return true;
}

static void AddBook() {
    Book book;
    book.id = NextBookId();
    book.stock = 1;
    if (!ShowBookForm(L"添加图书", book, false)) {
        return;
    }
    g_books.push_back(book);
    SaveBooks();
    AddLog("admin", "admin", "添加图书", book.title);
    MessageBoxW(GetHWnd(), L"图书添加成功。", L"提示", MB_OK | MB_ICONINFORMATION);
}

static void EditBook() {
    if (selectedRow < 0 || selectedRow >= static_cast<int>(g_books.size())) {
        MessageBoxW(GetHWnd(), L"请先选择图书。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    Book copy = g_books[selectedRow];
    if (!ShowBookForm(L"修改图书", copy, true)) {
        return;
    }
    g_books[selectedRow].title = copy.title;
    g_books[selectedRow].author = copy.author;
    g_books[selectedRow].publisher = copy.publisher;
    g_books[selectedRow].category = copy.category;
    g_books[selectedRow].stock = copy.stock;
    SaveBooks();
    AddLog("admin", "admin", "修改图书", copy.title);
    MessageBoxW(GetHWnd(), L"图书修改成功。", L"提示", MB_OK | MB_ICONINFORMATION);
}

static void DeleteBook() {
    if (selectedRow < 0 || selectedRow >= static_cast<int>(g_books.size())) {
        MessageBoxW(GetHWnd(), L"请先选择图书。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    if (g_books[selectedRow].borrowed > 0) {
        MessageBoxW(GetHWnd(), L"该图书仍有借出记录，不能删除。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    if (MessageBoxW(GetHWnd(), L"确定删除当前图书吗？", L"确认", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        AddLog("admin", "admin", "删除图书", g_books[selectedRow].title);
        g_books.erase(g_books.begin() + selectedRow);
        selectedRow = -1;
        SaveBooks();
    }
}

static void SearchBooks() {
    AskText(L"搜索图书", L"请输入关键词", searchKey, searchKey);
    page = 0;
    selectedRow = -1;
    RefreshRows();
}

static void FilterCategory() {
    AskText(L"分类筛选", L"分类（输入“全部”显示所有）", categoryFilter, categoryFilter.empty() ? L"全部" : categoryFilter);
    if (categoryFilter.empty()) {
        categoryFilter = L"全部";
    }
    page = 0;
    selectedRow = -1;
    RefreshRows();
}

static void DrawScreen(bool admin) {
    setbkcolor(CLR_BG);
    cleardevice();
    DrawHeaderPanel(admin ? L"图书管理" : L"图书查询");

    int by = 86;
    DrawBackButton(20, by);
    if (admin) {
        DrawButton(150, by, 105, 40, L"添加");
        DrawButton(270, by, 105, 40, L"修改", selectedRow != -1);
        DrawButton(390, by, 105, 40, L"删除", selectedRow != -1, CLR_DANGER);
    }
    DrawButton(660, by, 100, 40, L"搜索");
    DrawButton(775, by, 100, 40, L"分类");
    DrawButton(890, by, 100, 40, L"排序");
    DrawButton(1000, by, 80, 40, L"重置", true, CLR_SUCCESS);

    std::wstring sortText = sortMode == 0 ? L"ID" : (sortMode == 1 ? L"书名" : (sortMode == 2 ? L"可借库存" : L"热门度"));
    std::wstring info = L"搜索：" + (searchKey.empty() ? L"全部" : searchKey) + L"    分类：" + categoryFilter + L"    排序：" + sortText + L"    低库存显示红色";
    settextcolor(CLR_TEXT_LIGHT);
    setbkmode(TRANSPARENT);
    settextstyle_w(17, 0, L"微软雅黑");
    outtextxy_w(22, 140, info);

    DrawBookTable(20, 170);
    DrawPagination(20, WIN_H - 38, page, totalPage);
}

static void Loop(bool admin) {
    EnsureGraphInitialized();
    EnsureBaseDataFiles();
    LoadBooks();
    RefreshRows();
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        DrawScreen(admin);
        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x;
            int y = m.y;
            int by = 86;
            if (PtInRect(x, y, 20, by, 105, 40)) {
                return;
            }
            if (admin && PtInRect(x, y, 150, by, 105, 40)) {
                AddBook();
                LoadBooks();
                RefreshRows();
            }
            if (admin && PtInRect(x, y, 270, by, 105, 40)) {
                EditBook();
                LoadBooks();
                RefreshRows();
            }
            if (admin && PtInRect(x, y, 390, by, 105, 40)) {
                DeleteBook();
                LoadBooks();
                RefreshRows();
            }
            if (PtInRect(x, y, 660, by, 100, 40)) {
                SearchBooks();
            }
            if (PtInRect(x, y, 775, by, 100, 40)) {
                FilterCategory();
            }
            if (PtInRect(x, y, 890, by, 100, 40)) {
                sortMode = (sortMode + 1) % 4;
                RefreshRows();
            }
            if (PtInRect(x, y, 1000, by, 80, 40)) {
                searchKey.clear();
                categoryFilter = L"全部";
                sortMode = 0;
                selectedRow = -1;
                page = 0;
                RefreshRows();
            }
            if (x >= 20 && x <= 1040 && y >= 170 + ROW_H && y <= 170 + ROW_H * (PAGE_SIZE + 1)) {
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

void bookUIMain() {
    SetWindowTextW(GetHWnd(), L"DUT Library System - 图书管理");
    Loop(true);
}

void bookQueryUIMain() {
    SetWindowTextW(GetHWnd(), L"DUT Library System - 图书查询");
    Loop(false);
}
