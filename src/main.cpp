#include "ui_utils.h"
#include "book_ui.h"
#include "borrow_ui.h"
#include "log_ui.h"
#include "login.h"
#include "reader_ui.h"
#include "stat_ui.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

struct MenuCard {
    int x;
    int y;
    int w;
    int h;
    std::wstring title;
    std::wstring desc;
};

static IMAGE g_menuBg;
static bool g_hasMenuBg = false;

static void LoadMenuBg() {
    if (std::filesystem::exists("picture/bgImg.png")) {
        loadimage(&g_menuBg, L"picture/bgImg.png", WIN_W, WIN_H);
        g_hasMenuBg = true;
    } else {
        g_hasMenuBg = false;
    }
}

static POINT MousePoint() {
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(GetHWnd(), &p);
    return p;
}

static void DrawCard(const MenuCard& card, bool hover) {
    setfillcolor(hover ? RGB(235, 245, 255) : WHITE);
    setlinecolor(hover ? CLR_PRIMARY : RGB(220, 230, 245));
    solidroundrect(card.x, card.y, card.x + card.w, card.y + card.h, 16, 16);
    roundrect(card.x, card.y, card.x + card.w, card.y + card.h, 16, 16);

    setfillcolor(CLR_PRIMARY);
    solidroundrect(card.x, card.y, card.x + 8, card.y + card.h, 8, 8);

    setbkmode(TRANSPARENT);
    settextcolor(RGB(40, 80, 140));
    settextstyle_w(24, 0, L"微软雅黑");
    outtextxy_w(card.x + 28, card.y + 18, card.title);

    settextcolor(CLR_TEXT_LIGHT);
    settextstyle_w(15, 0, L"微软雅黑");
    outtextxy_w(card.x + 28, card.y + 55, card.desc);
}

static void DrawHeader(bool isAdmin, const std::string& userName) {
    // 菜单页顶部卡片缩小，不再占用大面积白色背景。
    setfillcolor(WHITE);
    setlinecolor(RGB(220, 230, 245));
    solidroundrect(300, 34, 800, 138, 24, 24);
    roundrect(300, 34, 800, 138, 24, 24);

    setbkmode(TRANSPARENT);
    settextcolor(RGB(40, 80, 140));
    settextstyle_w(34, 0, L"微软雅黑");
    std::wstring title = L"DUT Library System";
    outtextxy_w((WIN_W - textwidth_w(title)) / 2, 50, title);

    settextcolor(CLR_TEXT_DARK);
    settextstyle_w(18, 0, L"微软雅黑");
    std::wstring sub = isAdmin ? L"管理员控制中心" : L"读者服务中心";
    outtextxy_w((WIN_W - textwidth_w(sub)) / 2, 90, sub);

    settextcolor(CLR_TEXT_LIGHT);
    settextstyle_w(15, 0, L"微软雅黑");
    std::wstring current = L"当前用户：" + s2ws(userName);
    outtextxy_w((WIN_W - textwidth_w(current)) / 2, 116, current);
}


static void BackupData() {
    try {
        EnsureBaseDataFiles();
        std::filesystem::create_directories("backup");
        std::string folder = "backup/data_" + nowDateTime();
        std::filesystem::create_directories(folder);
        for (const auto& p : std::filesystem::directory_iterator("data")) {
            if (p.is_regular_file()) {
                std::filesystem::copy_file(
                    p.path(),
                    std::filesystem::path(folder) / p.path().filename(),
                    std::filesystem::copy_options::overwrite_existing
                );
            }
        }
        MessageBoxW(GetHWnd(), L"备份成功！已保存到 backup 文件夹。", L"提示", MB_OK | MB_ICONINFORMATION);
    } catch (...) {
        MessageBoxW(GetHWnd(), L"备份失败，请检查文件权限。", L"错误", MB_OK | MB_ICONERROR);
    }
}

void mainMenu(bool isAdmin, int readerId, const std::string& userName, const std::string& roleName) {
    EnsureGraphInitialized();
    EnsureBaseDataFiles();
    LoadMenuBg();
    SetWindowTextW(GetHWnd(), L"DUT Library System");

    ExMessage msg;
    while (true) {
        POINT mp = MousePoint();
        BeginBatchDraw();
        cleardevice();

        if (g_hasMenuBg) {
            putimage(0, 0, &g_menuBg);
        } else {
            GradientRect(0, 0, WIN_W, WIN_H, RGB(232, 242, 255), RGB(245, 248, 252));
        }

        setfillcolor(RGB(255, 255, 255));
        setlinecolor(RGB(225, 232, 245));
        solidroundrect(160, 178, 940, 635, 28, 28);
        roundrect(160, 178, 940, 635, 28, 28);

        DrawHeader(isAdmin, userName);

        if (isAdmin) {
            std::vector<MenuCard> cards = {
                { 205, 220, 300, 85, L"图书管理", L"管理图书信息 / 搜索分类排序" },
                { 555, 220, 300, 85, L"借阅管理", L"借还续借 / 逾期自动标红" },
                { 205, 330, 300, 85, L"读者管理", L"维护读者账号 / 状态管理" },
                { 555, 330, 300, 85, L"统计分析", L"库存借阅统计 / 热门排行" },
                { 205, 440, 300, 85, L"操作日志", L"查看增删改借还等操作记录" },
                { 555, 440, 300, 85, L"数据备份", L"备份当前 CSV 数据文件" },
                { 380, 550, 300, 68, L"退出系统", L"关闭 DUT Library System" }
            };
            for (const auto& card : cards) {
                DrawCard(card, PtInRect(mp.x, mp.y, card.x, card.y, card.w, card.h));
            }
        } else {
            std::vector<MenuCard> cards = {
                { 235, 250, 300, 90, L"查询图书", L"搜索馆藏图书和库存情况" },
                { 565, 250, 300, 90, L"我的借阅", L"查看个人借阅、归还和逾期状态" },
                { 235, 380, 300, 90, L"修改密码", L"在一个表单窗口中完成修改" },
                { 565, 380, 300, 90, L"退出系统", L"关闭 DUT Library System" }
            };
            for (const auto& card : cards) {
                DrawCard(card, PtInRect(mp.x, mp.y, card.x, card.y, card.w, card.h));
            }
        }
        EndBatchDraw();

        while (peekmessage(&msg, EX_MOUSE, true)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int x = msg.x;
                int y = msg.y;
                if (isAdmin) {
                    if (PtInRect(x, y, 205, 220, 300, 85)) {
                        bookUIMain();
                    } else if (PtInRect(x, y, 555, 220, 300, 85)) {
                        borrowUIMain();
                    } else if (PtInRect(x, y, 205, 330, 300, 85)) {
                        readerUIMain();
                    } else if (PtInRect(x, y, 555, 330, 300, 85)) {
                        statUIMain();
                    } else if (PtInRect(x, y, 205, 440, 300, 85)) {
                        logUIMain();
                    } else if (PtInRect(x, y, 555, 440, 300, 85)) {
                        BackupData();
                    } else if (PtInRect(x, y, 380, 550, 300, 68)) {
                        closegraph();
                        exit(0);
                    }
                } else {
                    if (PtInRect(x, y, 235, 250, 300, 90)) {
                        bookQueryUIMain();
                    } else if (PtInRect(x, y, 565, 250, 300, 90)) {
                        borrowMyRecordsUIMain(readerId);
                    } else if (PtInRect(x, y, 235, 380, 300, 90)) {
                        ChangePasswordUI(readerId);
                    } else if (PtInRect(x, y, 565, 380, 300, 90)) {
                        closegraph();
                        exit(0);
                    }
                }
            }
        }
        Sleep(10);
    }
}

int main() {
    EnsureBaseDataFiles();
    loginMain();
    return 0;
}
