#include "log_ui.h"
#include "ui_utils.h"

#include <sstream>
#include <string>
#include <vector>

struct LogItem {
    std::string time;
    std::string user;
    std::string role;
    std::string action;
    std::string detail;

    void fromCSV(const std::string& line) {
        std::stringstream ss(line);
        std::getline(ss, time, ',');
        std::getline(ss, user, ',');
        std::getline(ss, role, ',');
        std::getline(ss, action, ',');
        std::getline(ss, detail, ',');
    }

    std::string toCSV() const {
        return time + "," + user + "," + role + "," + action + "," + detail;
    }
};

static std::vector<LogItem> g_logs;

static void LoadLogs() {
    LoadCSV("data/logs.csv", g_logs, true);
}

static void SaveLogs() {
    SaveCSV("data/logs.csv", g_logs, "time,user,role,action,detail");
}

static int CalcLogTotalPage() {
    int totalPage = (static_cast<int>(g_logs.size()) + PAGE_SIZE - 1) / PAGE_SIZE;

    if (totalPage <= 0) {
        totalPage = 1;
    }

    return totalPage;
}

void AddLog(
    const std::string& user,
    const std::string& role,
    const std::string& action,
    const std::string& detail
) {
    EnsureBaseDataFiles();
    LoadLogs();
    g_logs.push_back({ nowDateTime(), user, role, action, detail });
    SaveLogs();
}

void logUIMain() {
    EnsureGraphInitialized();
    EnsureBaseDataFiles();
    SetWindowTextW(GetHWnd(), L"DUT Library System - 操作日志");
    LoadLogs();

    int page = 0;
    int totalPage = CalcLogTotalPage();

    ExMessage m;

    while (true) {
        BeginBatchDraw();

        setbkcolor(CLR_BG);
        cleardevice();

        DrawTitle(L"操作日志");

        DrawBackButton(20, 86);
        DrawButton(980, 86, 90, 38, L"刷新", true, CLR_SUCCESS);

        std::vector<std::wstring> headers = {
            L"时间",
            L"用户",
            L"身份",
            L"操作",
            L"详情"
        };

        std::vector<int> widths = {
            170,
            120,
            100,
            140,
            460
        };

        DrawTableHeader(40, 160, headers, widths);

        int start = page * PAGE_SIZE;

        for (int i = 0; i < PAGE_SIZE && start + i < static_cast<int>(g_logs.size()); ++i) {
            const auto& log = g_logs[start + i];

            int rowY = 160 + ROW_H * (i + 1);
            int x = 40;

            DrawCell(x, rowY, widths[0], s2ws(log.time));
            x += widths[0];

            DrawCell(x, rowY, widths[1], s2ws(log.user));
            x += widths[1];

            DrawCell(x, rowY, widths[2], s2ws(log.role));
            x += widths[2];

            DrawCell(x, rowY, widths[3], s2ws(log.action));
            x += widths[3];

            DrawCell(x, rowY, widths[4], s2ws(log.detail));
        }

        DrawPagination(40, WIN_H - 38, page, totalPage);

        EndBatchDraw();

        m = getmessage(EX_MOUSE);

        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x;
            int y = m.y;

            if (PtInRect(x, y, 20, 86, 105, 40)) {
                return;
            }

            if (PtInRect(x, y, 980, 86, 90, 38)) {
                LoadLogs();

                totalPage = CalcLogTotalPage();

                if (page >= totalPage) {
                    page = totalPage - 1;
                }

                if (page < 0) {
                    page = 0;
                }

                continue;
            }

            int prevX = 0;
            int nextX = 0;

            GetPaginationButtonPos(prevX, nextX);

            if (PtInRect(x, y, prevX, WIN_H - 46, 90, 34) && page > 0) {
                --page;
                continue;
            }

            if (PtInRect(x, y, nextX, WIN_H - 46, 90, 34) && page + 1 < totalPage) {
                ++page;
                continue;
            }
        }
    }
}
