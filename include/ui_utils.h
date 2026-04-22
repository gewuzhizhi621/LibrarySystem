#pragma once
#include <graphics.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <codecvt>
#include <locale>
#include <algorithm>

#pragma comment(lib, "EasyXw.lib")

// ---------- 全局常量 ----------
constexpr int WIN_W = 1100;
constexpr int WIN_H = 750;
constexpr int ROW_H = 36;
constexpr int COL_W = 140;
constexpr int PAGE_SIZE = 12;

constexpr COLORREF CLR_BG = RGB(245, 245, 250);
constexpr COLORREF CLR_PRIMARY = RGB(70, 130, 200);
constexpr COLORREF CLR_SECONDARY = RGB(100, 180, 220);
constexpr COLORREF CLR_ACCENT = RGB(255, 140, 0);
constexpr COLORREF CLR_TEXT_DARK = RGB(40, 40, 60);
constexpr COLORREF CLR_TEXT_LIGHT = RGB(120, 120, 140);

// ---------- 通用字符串转换 ----------
inline std::wstring s2ws(const std::string& str) {
    if (str.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    wstr.pop_back(); // remove null terminator
    return wstr;
}

// 确保图形库只初始化一次，避免多次 initgraph 导致窗口重建闪烁
inline void EnsureGraphInitialized() {
    static bool _graph_inited = false;
    if (!_graph_inited) {
        initgraph(WIN_W, WIN_H);
        _graph_inited = true;
    }
}

inline std::string ws2s(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], len, nullptr, nullptr);
    str.pop_back();
    return str;
}

inline std::wstring timeToWstring(time_t t) {
    if (t == 0) return L"";
    std::wostringstream wos;
    tm tm_;
    localtime_s(&tm_, &t);
    wos << std::put_time(&tm_, L"%Y-%m-%d");
    return wos.str();
}

inline time_t stringToTime(const std::string& s) {
    std::tm tm = {};
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    return mktime(&tm);
}

inline std::string timeToString(time_t t) {
    if (t == 0) return "";
    std::ostringstream os;
    tm tm_;
    localtime_s(&tm_, &t);
    os << std::put_time(&tm_, "%Y-%m-%d");
    return os.str();
}

// ---------- 美化控件 ----------
inline void DrawRoundedRect(int x1, int y1, int x2, int y2, int radius, COLORREF fillColor, COLORREF borderColor = BLACK, int borderWidth = 1) {
    setfillcolor(fillColor);
    setlinecolor(borderColor);
    setlinestyle(PS_SOLID, borderWidth);
    fillroundrect(x1, y1, x2, y2, radius, radius);
}

inline void DrawShadowRect(int x1, int y1, int x2, int y2, int radius, COLORREF fillColor) {
    setfillcolor(RGB(180, 180, 180));
    solidroundrect(x1 + 3, y1 + 3, x2 + 3, y2 + 3, radius, radius);
    DrawRoundedRect(x1, y1, x2, y2, radius, fillColor, CLR_TEXT_LIGHT, 1);
}

inline void DrawButton(int x, int y, int w, int h, const std::wstring& text, bool enabled = true, bool hover = false) {
    COLORREF bg = enabled ? (hover ? CLR_SECONDARY : CLR_PRIMARY) : RGB(200, 200, 200);
    DrawShadowRect(x, y, x + w, y + h, 8, bg);
    setbkmode(TRANSPARENT);
    settextcolor(enabled ? WHITE : RGB(150, 150, 150));
    settextstyle(20, 0, L"微软雅黑");
    int tw = textwidth(text.c_str());
    int th = textheight(text.c_str());
    outtextxy(x + (w - tw) / 2, y + (h - th) / 2, text.c_str());
}

inline void DrawInputBox(int x, int y, int w, int h, const std::wstring& text, bool active, int cursorPos) {
    COLORREF bg = active ? RGB(255, 255, 240) : WHITE;
    COLORREF border = active ? CLR_PRIMARY : CLR_TEXT_LIGHT;
    DrawRoundedRect(x, y, x + w, y + h, 5, bg, border, 2);
    setbkmode(TRANSPARENT);
    settextcolor(CLR_TEXT_DARK);
    settextstyle(18, 0, L"微软雅黑");
    outtextxy(x + 8, y + (h - 20) / 2, text.c_str());
    if (active && cursorPos >= 0 && clock() % 1000 < 500) {
        std::wstring sub = text.substr(0, cursorPos);
        int tw = textwidth(sub.c_str());
        setlinecolor(BLACK);
        line(x + 8 + tw, y + 6, x + 8 + tw, y + h - 8);
    }
}

inline bool PtInRect(int x, int y, int left, int top, int w, int h) {
    return x >= left && x <= left + w && y >= top && y <= top + h;
}

// ---------- CSV 读写模板 ----------
template<typename T>
void LoadCSV(const std::string& path, std::vector<T>& vec, bool hasHeader = true) {
    vec.clear();
    std::ifstream fin(path);
    if (!fin) return;
    std::string line;
    if (hasHeader) std::getline(fin, line);
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        T item;
        item.fromCSV(line);
        vec.push_back(std::move(item));
    }
}

template<typename T>
void SaveCSV(const std::string& path, const std::vector<T>& vec, const std::string& header) {
    std::ofstream fout(path);
    if (!fout) return;
    fout << header << '\n';
    for (const auto& item : vec) fout << item.toCSV() << '\n';
}

// ---------- 绘制表头 ----------
inline void DrawTableHeader(int x, int y, const std::vector<std::wstring>& headers, const std::vector<int>& widths) {
    int curX = x;
    for (size_t i = 0; i < headers.size(); ++i) {
        DrawRoundedRect(curX, y, curX + widths[i], y + ROW_H, 0, RGB(230, 240, 250), CLR_TEXT_LIGHT);
        setbkmode(TRANSPARENT);
        settextcolor(CLR_TEXT_DARK);
        settextstyle(18, 0, L"微软雅黑");
        outtextxy(curX + 5, y + 5, headers[i].c_str());
        curX += widths[i];
    }
}

// 分页控件
inline void DrawPagination(int x, int y, int page, int totalPage) {
    std::wstring info = L"第 " + std::to_wstring(page + 1) + L" / " + std::to_wstring(totalPage) + L" 页";
    settextstyle(18, 0, L"微软雅黑");
    settextcolor(CLR_TEXT_DARK);
    outtextxy(x, y, info.c_str());
    DrawButton(x + 150, y - 8, 80, 30, L"上一页", page > 0);
    DrawButton(x + 240, y - 8, 80, 30, L"下一页", (page + 1) < totalPage);
}