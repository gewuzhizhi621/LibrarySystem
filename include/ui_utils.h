#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <graphics.h>
#include <windows.h>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

constexpr int WIN_W = 1100;
constexpr int WIN_H = 750;
constexpr int ROW_H = 42;
constexpr int PAGE_SIZE = 10;

constexpr COLORREF CLR_BG = RGB(245, 248, 252);
constexpr COLORREF CLR_PRIMARY = RGB(50, 125, 220);
constexpr COLORREF CLR_DANGER = RGB(220, 80, 80);
constexpr COLORREF CLR_SUCCESS = RGB(70, 170, 120);
constexpr COLORREF CLR_WARNING = RGB(240, 150, 60);
constexpr COLORREF CLR_TEXT_DARK = RGB(40, 45, 55);
constexpr COLORREF CLR_TEXT_LIGHT = RGB(120, 135, 155);
constexpr COLORREF CLR_LINE = RGB(214, 223, 236);

inline std::wstring s2ws(const std::string& str) {
    if (str.empty()) {
        return L"";
    }
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (len <= 0) {
        return L"";
    }
    std::wstring w(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &w[0], len);
    if (!w.empty() && w.back() == L'\0') {
        w.pop_back();
    }
    return w;
}

inline std::string ws2s(const std::wstring& wstr) {
    if (wstr.empty()) {
        return "";
    }
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) {
        return "";
    }
    std::string s(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &s[0], len, nullptr, nullptr);
    if (!s.empty() && s.back() == '\0') {
        s.pop_back();
    }
    return s;
}

inline void settextstyle_w(int h, int w, const std::wstring& font) {
    settextstyle(h, w, font.c_str());
}

inline int textwidth_w(const std::wstring& text) {
    return textwidth(text.c_str());
}

inline int textheight_w(const std::wstring& text) {
    return textheight(text.c_str());
}

inline void outtextxy_w(int x, int y, const std::wstring& text) {
    outtextxy(x, y, text.c_str());
}

inline bool PtInRect(int x, int y, int left, int top, int width, int height) {
    return x >= left && x <= left + width && y >= top && y <= top + height;
}

inline void EnsureGraphInitialized() {
    static bool inited = false;
    if (!inited) {
        initgraph(WIN_W, WIN_H);
        BeginBatchDraw();
        inited = true;
    }
}

inline std::string timeToString(time_t t) {
    if (t == 0) {
        return "";
    }
    tm tmv{};
    localtime_s(&tmv, &t);
    std::ostringstream os;
    os << std::put_time(&tmv, "%Y-%m-%d");
    return os.str();
}

inline std::wstring timeToWstring(time_t t) {
    return s2ws(timeToString(t));
}

inline time_t stringToTime(const std::string& s) {
    std::string t = s;

    while (!t.empty() && (t.back() == '\r' || t.back() == '\n' || t.back() == ' ' || t.back() == '\t')) {
        t.pop_back();
    }

    while (!t.empty() && (t.front() == ' ' || t.front() == '\t')) {
        t.erase(t.begin());
    }

    if (t.size() >= 3 &&
        static_cast<unsigned char>(t[0]) == 0xEF &&
        static_cast<unsigned char>(t[1]) == 0xBB &&
        static_cast<unsigned char>(t[2]) == 0xBF) {
        t.erase(0, 3);
    }

    if (t.empty() || t == "0" || t == "未归还") {
        return 0;
    }

    int year = 0;
    int month = 0;
    int day = 0;

    if (sscanf_s(t.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        return 0;
    }

    if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        return 0;
    }

    std::tm tmv{};
    tmv.tm_year = year - 1900;
    tmv.tm_mon = month - 1;
    tmv.tm_mday = day;
    tmv.tm_hour = 12;
    tmv.tm_min = 0;
    tmv.tm_sec = 0;
    tmv.tm_isdst = -1;

    return mktime(&tmv);
}

inline std::string nowDateTime() {
    time_t t = time(nullptr);
    tm tmv{};
    localtime_s(&tmv, &t);
    std::ostringstream os;
    os << std::put_time(&tmv, "%Y-%m-%d_%H-%M-%S");
    return os.str();
}

inline void DrawButton(
    int x,
    int y,
    int width,
    int height,
    const std::wstring& text,
    bool enabled = true,
    COLORREF color = CLR_PRIMARY
) {
    COLORREF bg = enabled ? color : RGB(200, 205, 215);
    COLORREF border = enabled ? RGB(40, 100, 190) : RGB(170, 175, 185);
    setfillcolor(bg);
    setlinecolor(border);
    solidroundrect(x, y, x + width, y + height, 10, 10);
    roundrect(x, y, x + width, y + height, 10, 10);

    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle_w(20, 0, L"微软雅黑");
    int tw = textwidth_w(text);
    int th = textheight_w(text);
    outtextxy_w(x + (width - tw) / 2, y + (height - th) / 2, text);
}

inline void DrawBackButton(int x = 20, int y = 86) {
    DrawButton(x, y, 105, 40, L"← 返回");
}

inline void DrawHeaderPanel(const std::wstring& title) {
    // 紧凑标题栏：只占顶部一小块区域，不再铺满整行，避免白色区域过大。
    setfillcolor(WHITE);
    setlinecolor(RGB(220, 230, 245));
    solidroundrect(360, 12, WIN_W - 360, 76, 18, 18);
    roundrect(360, 12, WIN_W - 360, 76, 18, 18);

    setbkmode(TRANSPARENT);
    settextcolor(CLR_PRIMARY);
    settextstyle_w(28, 0, L"微软雅黑");
    int tw = textwidth_w(title);
    outtextxy_w((WIN_W - tw) / 2, 18, title);

    settextcolor(RGB(120, 135, 155));
    settextstyle_w(14, 0, L"微软雅黑");
    std::wstring sub = L"DUT Library System";
    int sw = textwidth_w(sub);
    outtextxy_w((WIN_W - sw) / 2, 52, sub);
}

inline void DrawTitle(const std::wstring& title) {
    DrawHeaderPanel(title);
}

inline void DrawTitle(const wchar_t* title) {
    DrawHeaderPanel(title ? std::wstring(title) : L"");
}

inline void DrawTitle(const std::string& title) {
    DrawHeaderPanel(s2ws(title));
}


inline void DrawTableHeader(
    int x,
    int y,
    const std::vector<std::wstring>& headers,
    const std::vector<int>& widths
) {
    int cur = x;
    for (size_t i = 0; i < headers.size() && i < widths.size(); ++i) {
        setfillcolor(RGB(225, 238, 252));
        setlinecolor(RGB(190, 205, 225));
        solidrectangle(cur, y, cur + widths[i], y + ROW_H);
        rectangle(cur, y, cur + widths[i], y + ROW_H);

        setbkmode(TRANSPARENT);
        settextcolor(CLR_TEXT_DARK);
        settextstyle_w(18, 0, L"微软雅黑");
        outtextxy_w(cur + 8, y + 10, headers[i]);
        cur += widths[i];
    }
}

inline void DrawCell(
    int x,
    int y,
    int width,
    const std::wstring& text,
    COLORREF bg = WHITE,
    COLORREF color = CLR_TEXT_DARK
) {
    setfillcolor(bg);
    setlinecolor(CLR_LINE);
    solidrectangle(x, y, x + width, y + ROW_H);
    rectangle(x, y, x + width, y + ROW_H);

    setbkmode(TRANSPARENT);
    settextcolor(color);
    settextstyle_w(17, 0, L"微软雅黑");

    std::wstring show = text;
    while (textwidth_w(show) > width - 12 && show.size() > 2) {
        show.pop_back();
    }
    if (show != text && show.size() > 2) {
        show.pop_back();
        show += L"...";
    }
    outtextxy_w(x + 6, y + 10, show);
}

inline void GetPaginationButtonPos(int& prevX, int& nextX) {
    const int infoW = 110;
    const int btnW = 90;
    const int gap1 = 35;
    const int gap2 = 15;
    const int totalW = infoW + gap1 + btnW + gap2 + btnW;
    const int startX = (WIN_W - totalW) / 2;

    prevX = startX + infoW + gap1;
    nextX = prevX + btnW + gap2;
}

inline void DrawPagination(int x, int y, int page, int totalPage) {
    const int infoW = 110;
    const int btnW = 90;
    const int btnH = 34;
    const int gap1 = 35;
    const int gap2 = 15;
    const int totalW = infoW + gap1 + btnW + gap2 + btnW;
    const int startX = (WIN_W - totalW) / 2;

    int infoX = startX;
    int prevX = 0;
    int nextX = 0;
    GetPaginationButtonPos(prevX, nextX);

    setbkmode(TRANSPARENT);
    settextcolor(CLR_TEXT_DARK);
    settextstyle_w(18, 0, L"微软雅黑");

    std::wstring info = L"第 " + std::to_wstring(page + 1) + L" / " + std::to_wstring(totalPage) + L" 页";
    outtextxy_w(infoX, y, info);

    DrawButton(prevX, y - 8, btnW, btnH, L"上一页", page > 0);
    DrawButton(nextX, y - 8, btnW, btnH, L"下一页", page + 1 < totalPage);
}

template<typename T>
inline void LoadCSV(const std::string& path, std::vector<T>& vec, bool hasHeader = true) {
    vec.clear();
    std::ifstream fin(path);
    if (!fin) {
        return;
    }
    std::string line;
    if (hasHeader) {
        std::getline(fin, line);
    }
    while (std::getline(fin, line)) {
        if (line.empty()) {
            continue;
        }
        try {
            T item;
            item.fromCSV(line);
            vec.push_back(item);
        }
        catch (...) {
        }
    }
}

template<typename T>
inline void SaveCSV(const std::string& path, const std::vector<T>& vec, const std::string& header) {
    std::filesystem::path p(path);
    if (!p.parent_path().empty()) {
        std::filesystem::create_directories(p.parent_path());
    }
    std::ofstream fout(path);
    if (!fout) {
        return;
    }
    fout.write("\xEF\xBB\xBF", 3);
    fout << header << "\n";
    for (const auto& item : vec) {
        fout << item.toCSV() << "\n";
    }
}

inline bool AskText(const std::wstring& title, const std::wstring& prompt, std::wstring& result, const std::wstring& def = L"") {
    wchar_t buf[512]{};
    wcsncpy_s(buf, def.c_str(), 511);
    bool ok = InputBox(buf, 512, prompt.c_str(), title.c_str(), def.c_str());
    if (ok) {
        result = buf;
    }
    return ok;
}

inline bool AskInt(const std::wstring& title, const std::wstring& prompt, int& value, int def = 0) {
    std::wstring input;
    if (!AskText(title, prompt, input, std::to_wstring(def))) {
        return false;
    }
    try {
        value = std::stoi(input);
        return true;
    }
    catch (...) {
        MessageBoxW(GetHWnd(), L"请输入正确数字！", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }
}

inline void GradientRect(int x, int y, int width, int height, COLORREF c1, COLORREF c2) {
    for (int i = 0; i < height; ++i) {
        double t = static_cast<double>(i) / (std::max)(1, height - 1);
        int r = static_cast<int>(GetRValue(c1) * (1 - t) + GetRValue(c2) * t);
        int g = static_cast<int>(GetGValue(c1) * (1 - t) + GetGValue(c2) * t);
        int b = static_cast<int>(GetBValue(c1) * (1 - t) + GetBValue(c2) * t);
        setlinecolor(RGB(r, g, b));
        line(x, y + i, x + width, y + i);
    }
}

struct FormField {
    std::wstring label;
    std::wstring value;
    bool password = false;
    bool readOnly = false;
};

struct FormDialogState {
    std::vector<FormField>* fields = nullptr;
    std::vector<HWND> edits;
    HFONT font = nullptr;
    bool done = false;
    bool ok = false;
    int width = 480;
};

inline LRESULT CALLBACK LibraryFormProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    FormDialogState* state = reinterpret_cast<FormDialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return TRUE;
    }

    switch (msg) {
    case WM_CREATE:
    {
        state = reinterpret_cast<FormDialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (!state || !state->fields) {
            return -1;
        }

        HINSTANCE hInst = GetModuleHandleW(nullptr);
        int y = 18;
        int labelX = 22;
        int editX = 150;
        int editW = state->width - editX - 34;

        for (size_t i = 0; i < state->fields->size(); ++i) {
            const FormField& field = (*state->fields)[i];

            HWND hLabel = CreateWindowW(
                L"STATIC",
                field.label.c_str(),
                WS_CHILD | WS_VISIBLE,
                labelX,
                y + 7,
                120,
                24,
                hwnd,
                nullptr,
                hInst,
                nullptr
            );
            SendMessageW(hLabel, WM_SETFONT, reinterpret_cast<WPARAM>(state->font), TRUE);

            DWORD editStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
            if (field.password) {
                editStyle |= ES_PASSWORD;
            }

            HWND hEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                L"EDIT",
                field.value.c_str(),
                editStyle,
                editX,
                y,
                editW,
                30,
                hwnd,
                reinterpret_cast<HMENU>(static_cast<UINT_PTR>(1000 + static_cast<int>(i))),
                hInst,
                nullptr
            );
            SendMessageW(hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(state->font), TRUE);
            if (field.readOnly) {
                SendMessageW(hEdit, EM_SETREADONLY, TRUE, 0);
            }
            state->edits.push_back(hEdit);
            y += 44;
        }

        HWND hOk = CreateWindowW(
            L"BUTTON",
            L"确定",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
            state->width - 198,
            y + 10,
            72,
            30,
            hwnd,
            reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDOK)),
            hInst,
            nullptr
        );
        SendMessageW(hOk, WM_SETFONT, reinterpret_cast<WPARAM>(state->font), TRUE);

        HWND hCancel = CreateWindowW(
            L"BUTTON",
            L"取消",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
            state->width - 110,
            y + 10,
            72,
            30,
            hwnd,
            reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDCANCEL)),
            hInst,
            nullptr
        );
        SendMessageW(hCancel, WM_SETFONT, reinterpret_cast<WPARAM>(state->font), TRUE);

        if (!state->edits.empty()) {
            SetFocus(state->edits[0]);
        }
        return 0;
    }
    case WM_COMMAND:
        if (!state || !state->fields) {
            break;
        }
        if (LOWORD(wParam) == IDOK) {
            wchar_t buf[1024]{};
            for (size_t i = 0; i < state->edits.size(); ++i) {
                GetWindowTextW(state->edits[i], buf, 1024);
                (*state->fields)[i].value = buf;
            }
            state->ok = true;
            state->done = true;
            DestroyWindow(hwnd);
            return 0;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            state->ok = false;
            state->done = true;
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_CLOSE:
        if (state) {
            state->ok = false;
            state->done = true;
        }
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

inline bool ShowFormDialog(const std::wstring& title, std::vector<FormField>& fields, int width = 480) {
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = LibraryFormProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"DUT_LIBRARY_FORM_WINDOW";
        RegisterClassW(&wc);
        registered = true;
    }

    FormDialogState state;
    state.fields = &fields;
    state.width = width;
    state.font = CreateFontW(
        18,
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"微软雅黑"
    );

    HWND parent = GetHWnd();
    if (parent) {
        EnableWindow(parent, FALSE);
    }

    int height = 110 + static_cast<int>(fields.size()) * 44 + 56;
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"DUT_LIBRARY_FORM_WINDOW",
        title.c_str(),
        WS_CAPTION | WS_SYSMENU | WS_POPUPWINDOW,
        x,
        y,
        width,
        height,
        parent,
        nullptr,
        GetModuleHandleW(nullptr),
        &state
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg{};
    while (!state.done && GetMessageW(&msg, nullptr, 0, 0)) {
        if (!IsDialogMessageW(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    if (parent) {
        EnableWindow(parent, TRUE);
        SetActiveWindow(parent);
    }
    DeleteObject(state.font);
    return state.ok;
}

inline void EnsureTextFile(const std::string& path, const std::string& content) {
    std::filesystem::path p(path);
    if (!p.parent_path().empty()) {
        std::filesystem::create_directories(p.parent_path());
    }
    if (!std::filesystem::exists(p) || std::filesystem::file_size(p) == 0) {
        std::ofstream fout(path);
        fout << content;
    }
}

inline void EnsureBaseDataFiles() {
    std::filesystem::create_directories("data");
    std::filesystem::create_directories("backup");
    std::filesystem::create_directories("picture");

    EnsureTextFile(
        "data/admin_accounts.csv",
        "\xEF\xBB\xBF" "username,password,name,role\n"
    );

    EnsureTextFile(
        "data/books.csv",
        "\xEF\xBB\xBF" "id,title,author,publisher,category,stock,borrowed,totalBorrowed\n"
    );

    EnsureTextFile(
        "data/readers.csv",
        "\xEF\xBB\xBF" "id,name,contact,regDate,username,password,status\n"
    );

    EnsureTextFile(
        "data/borrow_records.csv",
        "\xEF\xBB\xBF" "bookId,bookTitle,readerId,readerName,borrowDate,dueDate,returnDate\n"
    );

    EnsureTextFile(
        "data/logs.csv",
        "\xEF\xBB\xBF" "time,user,role,action,detail\n"
    );
}
