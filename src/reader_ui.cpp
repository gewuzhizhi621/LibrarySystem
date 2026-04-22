#include "reader_ui.h"
#include "ui_utils.h"
#include <algorithm>
#include <vector>



// ---------- 全局数据 ----------
std::vector<Reader> g_readers;
static int page = 0, totalPage = 0;
static int selectedRow = -1;

// ---------- 数据加载与保存 ----------
void LoadReaders() {
    LoadCSV("data/readers.csv", g_readers, true);
    totalPage = (g_readers.size() + PAGE_SIZE - 1) / PAGE_SIZE;
    if (totalPage == 0) totalPage = 1;
}

void SaveReaders() {
    SaveCSV("data/readers.csv", g_readers, "id,name,contact,regDate");
}

// ---------- 绘制读者表格 ----------
static void DrawReaderTable(int x0, int y0) {
    std::vector<std::wstring> headers = { L"ID", L"姓名", L"联系方式", L"注册日期" };
    std::vector<int> widths = { 80, 200, 220, 180 };
    DrawTableHeader(x0, y0, headers, widths);

    int start = page * PAGE_SIZE;
    for (int i = 0; i < PAGE_SIZE && start + i < (int)g_readers.size(); ++i) {
        const Reader& r = g_readers[start + i];
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
        outtextxy(curX + 5, y + 8, std::to_wstring(r.id).c_str()); curX += widths[0];
        outtextxy(curX + 5, y + 8, s2ws(r.name).c_str()); curX += widths[1];
        outtextxy(curX + 5, y + 8, s2ws(r.contact).c_str()); curX += widths[2];
        outtextxy(curX + 5, y + 8, s2ws(r.regDate).c_str());
    }
}

// ---------- 添加读者对话框 ----------
static void AddReaderDialog() {
    std::wstring name, contact;
    std::vector<std::wstring*> fields = { &name, &contact };
    const wchar_t* labels[] = { L"姓名", L"联系方式" };
    int activeField = -1;
    std::vector<int> cursors(2, 0);
    std::wstring errorMsg;
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        // 对话框不清屏，仅绘制对话框背景覆盖当前界面以减少闪烁
        DrawShadowRect(240, 80, 760, 460, 12, RGB(255, 255, 255));
        settextstyle(26, 0, L"微软雅黑");
        settextcolor(CLR_TEXT_DARK);
        outtextxy(400, 60, L"添加读者");
        settextstyle(18, 0, L"微软雅黑");
        for (int i = 0; i < 2; ++i) {
            outtextxy(300, 140 + i * 60, labels[i]);
            DrawInputBox(420, 140 + i * 60, 280, 36, *fields[i], activeField == i, cursors[i]);
        }
        if (!errorMsg.empty()) {
            settextcolor(RED);
            outtextxy(420, 300, errorMsg.c_str());
        }
        DrawButton(420, 360, 100, 40, L"确定");
        DrawButton(540, 360, 100, 40, L"取消");
        EndBatchDraw();

        m = getmessage(EX_CHAR | EX_KEY | EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            activeField = -1;
            for (int i = 0; i < 2; ++i)
                if (PtInRect(m.x, m.y, 420, 140 + i * 60, 280, 36)) { activeField = i; break; }
            if (PtInRect(m.x, m.y, 420, 360, 100, 40)) {
                if (name.empty() || contact.empty()) {
                    errorMsg = L"姓名和联系方式不能为空！";
                    continue;
                }
                Reader r;
                r.id = g_readers.empty() ? 1 : g_readers.back().id + 1;
                r.name = ws2s(name);
                r.contact = ws2s(contact);
                // 获取当前日期作为注册日期
                time_t now = std::time(nullptr);
                r.regDate = timeToString(now);
                g_readers.push_back(r);
                SaveReaders();
                return;
            }
            if (PtInRect(m.x, m.y, 540, 360, 100, 40)) return;
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

// ---------- 删除选中读者 ----------
static void DeleteSelected() {
    if (selectedRow >= 0 && selectedRow < (int)g_readers.size()) {
        g_readers.erase(g_readers.begin() + selectedRow);
        SaveReaders();
        selectedRow = -1;
        if (page >= totalPage && page > 0) --page;
    }
}

// ---------- 主界面 ----------
void readerUIMain() {
    initgraph(WIN_W, WIN_H);
    SetWindowTextW(GetHWnd(), L"读者管理 - 图书馆系统");
    LoadReaders();
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();

        // 顶部工具栏
        DrawButton(20, 20, 100, 40, L"← 返回");
        DrawButton(140, 20, 100, 40, L"添加读者");
        DrawButton(260, 20, 100, 40, L"删除选中", selectedRow != -1);

        // 表格区域
        DrawReaderTable(30, 90);

        // 分页
        DrawPagination(30, WIN_H - 50, page, totalPage);

        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x, y = m.y;
            // 返回
            if (PtInRect(x, y, 20, 20, 100, 40)) return;
            // 添加读者
            if (PtInRect(x, y, 140, 20, 100, 40)) { AddReaderDialog(); LoadReaders(); }
            // 删除选中
            if (PtInRect(x, y, 260, 20, 100, 40) && selectedRow != -1) { DeleteSelected(); LoadReaders(); }
            // 表格点击选中
            if (x >= 30 && x <= 700 && y >= 90 + ROW_H && y <= 90 + ROW_H * (PAGE_SIZE + 1)) {
                int row = (y - 90) / ROW_H - 1;
                int idx = page * PAGE_SIZE + row;
                if (idx >= 0 && idx < (int)g_readers.size()) selectedRow = idx;
                else selectedRow = -1;
            }
            // 分页
            int pageBtnX = 30 + 150, pageBtnY = WIN_H - 58;
            if (PtInRect(x, y, pageBtnX, pageBtnY, 80, 30) && page > 0) --page;
            if (PtInRect(x, y, pageBtnX + 90, pageBtnY, 80, 30) && (page + 1) < totalPage) ++page;
        }
    }
}

// 为了让 borrow_ui.cpp 能够访问读者数据，提供全局函数
std::vector<Reader>& GetReaders() { return g_readers; }
void ReloadReaders() { LoadReaders(); }