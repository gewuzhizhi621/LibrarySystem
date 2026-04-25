#include "reader_ui.h"
#include "log_ui.h"
#include "ui_utils.h"

#include <algorithm>
#include <ctime>
#include <string>
#include <vector>

std::vector<Reader> g_readers;

static int page = 0;
static int totalPage = 1;
static int selectedRow = -1;
static std::wstring searchKey;
static std::vector<int> displayRows;

void LoadReaders() {
    LoadCSV("data/readers.csv", g_readers, true);
}

void SaveReaders() {
    SaveCSV("data/readers.csv", g_readers, "id,name,contact,regDate,username,password,status");
}

static int NextReaderId() {
    int maxId = 0;
    for (const auto& r : g_readers) {
        maxId = (std::max)(maxId, r.id);
    }
    return maxId + 1;
}

static bool MatchReader(const Reader& r, const std::string& key) {
    if (key.empty()) {
        return true;
    }
    return r.name.find(key) != std::string::npos ||
        r.contact.find(key) != std::string::npos ||
        r.username.find(key) != std::string::npos ||
        r.status.find(key) != std::string::npos ||
        std::to_string(r.id).find(key) != std::string::npos;
}

static void RefreshRows() {
    displayRows.clear();
    std::string key = ws2s(searchKey);
    for (int i = 0; i < static_cast<int>(g_readers.size()); ++i) {
        if (MatchReader(g_readers[i], key)) {
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

static void DrawReaderTable(int x, int y) {
    std::vector<std::wstring> headers = { L"ID", L"姓名", L"联系方式", L"注册日期", L"账号", L"状态" };
    std::vector<int> widths = { 60, 170, 220, 155, 220, 100 };
    DrawTableHeader(x, y, headers, widths);

    int start = page * PAGE_SIZE;
    for (int i = 0; i < PAGE_SIZE && start + i < static_cast<int>(displayRows.size()); ++i) {
        int idx = displayRows[start + i];
        const Reader& r = g_readers[idx];
        int rowY = y + ROW_H * (i + 1);
        int cur = x;
        COLORREF bg = idx == selectedRow ? RGB(230, 241, 255) : (r.status == "disabled" ? RGB(245, 245, 245) : WHITE);
        COLORREF statusColor = r.status == "disabled" ? CLR_DANGER : CLR_SUCCESS;

        DrawCell(cur, rowY, widths[0], std::to_wstring(r.id), bg); cur += widths[0];
        DrawCell(cur, rowY, widths[1], s2ws(r.name), bg); cur += widths[1];
        DrawCell(cur, rowY, widths[2], s2ws(r.contact), bg); cur += widths[2];
        DrawCell(cur, rowY, widths[3], s2ws(r.regDate), bg); cur += widths[3];
        DrawCell(cur, rowY, widths[4], s2ws(r.username), bg); cur += widths[4];
        DrawCell(cur, rowY, widths[5], r.status == "disabled" ? L"停用" : L"正常", bg, statusColor);
    }
}

static bool ShowReaderForm(const std::wstring& title, Reader& reader, bool isEdit) {
    std::vector<FormField> fields = {
        { L"姓名", s2ws(reader.name) },
        { L"联系方式", s2ws(reader.contact) },
        { L"账号", s2ws(reader.username) },
        { L"密码", s2ws(reader.password), true },
        { L"状态(normal/disabled)", s2ws(reader.status.empty() ? "normal" : reader.status) }
    };
    if (!ShowFormDialog(title, fields, 560)) {
        return false;
    }
    for (const auto& f : fields) {
        if (f.value.empty()) {
            MessageBoxW(GetHWnd(), L"请完整填写所有信息。", L"错误", MB_OK | MB_ICONERROR);
            return false;
        }
    }

    std::string newUser = ws2s(fields[2].value);
    for (int i = 0; i < static_cast<int>(g_readers.size()); ++i) {
        if ((!isEdit || i != selectedRow) && g_readers[i].username == newUser) {
            MessageBoxW(GetHWnd(), L"账号已存在，请更换。", L"错误", MB_OK | MB_ICONERROR);
            return false;
        }
    }

    reader.name = ws2s(fields[0].value);
    reader.contact = ws2s(fields[1].value);
    reader.username = newUser;
    reader.password = ws2s(fields[3].value);
    reader.status = ws2s(fields[4].value);
    if (reader.status != "disabled") {
        reader.status = "normal";
    }
    return true;
}

static void AddReader() {
    Reader r;
    r.id = NextReaderId();
    r.regDate = timeToString(time(nullptr));
    r.username = "reader" + std::to_string(r.id);
    r.password = "123456";
    r.status = "normal";
    if (!ShowReaderForm(L"添加读者", r, false)) {
        return;
    }
    g_readers.push_back(r);
    SaveReaders();
    AddLog("admin", "admin", "添加读者", r.name);
    MessageBoxW(GetHWnd(), L"读者添加成功。", L"提示", MB_OK | MB_ICONINFORMATION);
}

static void EditReader() {
    if (selectedRow < 0 || selectedRow >= static_cast<int>(g_readers.size())) {
        MessageBoxW(GetHWnd(), L"请先选择读者。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    Reader tmp = g_readers[selectedRow];
    if (!ShowReaderForm(L"修改读者", tmp, true)) {
        return;
    }
    g_readers[selectedRow] = tmp;
    SaveReaders();
    AddLog("admin", "admin", "修改读者", tmp.name);
    MessageBoxW(GetHWnd(), L"读者修改成功。", L"提示", MB_OK | MB_ICONINFORMATION);
}

static void DeleteReader() {
    if (selectedRow < 0 || selectedRow >= static_cast<int>(g_readers.size())) {
        MessageBoxW(GetHWnd(), L"请先选择读者。", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    if (MessageBoxW(GetHWnd(), L"确定删除当前读者吗？", L"确认", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        AddLog("admin", "admin", "删除读者", g_readers[selectedRow].name);
        g_readers.erase(g_readers.begin() + selectedRow);
        selectedRow = -1;
        SaveReaders();
    }
}

static void SearchReaders() {
    AskText(L"搜索读者", L"请输入关键词", searchKey, searchKey);
    page = 0;
    selectedRow = -1;
    RefreshRows();
}

void readerUIMain() {
    EnsureGraphInitialized();
    EnsureBaseDataFiles();
    SetWindowTextW(GetHWnd(), L"DUT Library System - 读者管理");
    LoadReaders();
    RefreshRows();
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();
        DrawHeaderPanel(L"读者管理");

        int by = 86;
        DrawBackButton(20, by);
        DrawButton(150, by, 105, 40, L"添加");
        DrawButton(270, by, 105, 40, L"修改", selectedRow != -1);
        DrawButton(390, by, 105, 40, L"删除", selectedRow != -1, CLR_DANGER);
        DrawButton(660, by, 100, 40, L"搜索");
        DrawButton(775, by, 100, 40, L"重置", true, CLR_SUCCESS);

        setbkmode(TRANSPARENT);
        settextcolor(CLR_TEXT_LIGHT);
        settextstyle_w(17, 0, L"微软雅黑");
        std::wstring info = L"搜索：" + (searchKey.empty() ? L"全部" : searchKey) + L"    停用账号会显示灰色";
        outtextxy_w(22, 140, info);

        DrawReaderTable(20, 170);
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
                AddReader();
                LoadReaders();
                RefreshRows();
            }
            if (PtInRect(x, y, 270, by, 105, 40)) {
                EditReader();
                LoadReaders();
                RefreshRows();
            }
            if (PtInRect(x, y, 390, by, 105, 40)) {
                DeleteReader();
                LoadReaders();
                RefreshRows();
            }
            if (PtInRect(x, y, 660, by, 100, 40)) {
                SearchReaders();
            }
            if (PtInRect(x, y, 775, by, 100, 40)) {
                searchKey.clear();
                page = 0;
                selectedRow = -1;
                RefreshRows();
            }
            if (x >= 20 && x <= 965 && y >= 170 + ROW_H && y <= 170 + ROW_H * (PAGE_SIZE + 1)) {
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

void ChangePasswordUI(int readerId) {
    LoadReaders();
    auto it = std::find_if(g_readers.begin(), g_readers.end(), [&](const Reader& r) { return r.id == readerId; });
    if (it == g_readers.end()) {
        MessageBoxW(GetHWnd(), L"读者不存在。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    std::vector<FormField> fields = {
        { L"原密码", L"", true },
        { L"新密码", L"", true },
        { L"确认新密码", L"", true }
    };
    if (!ShowFormDialog(L"修改密码", fields, 480)) {
        return;
    }
    if (ws2s(fields[0].value) != it->password) {
        MessageBoxW(GetHWnd(), L"原密码错误。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    if (fields[1].value.empty() || fields[1].value != fields[2].value) {
        MessageBoxW(GetHWnd(), L"两次输入的新密码不一致。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    it->password = ws2s(fields[1].value);
    SaveReaders();
    AddLog(it->username, "reader", "修改密码", "success");
    MessageBoxW(GetHWnd(), L"密码修改成功。", L"提示", MB_OK | MB_ICONINFORMATION);
}
