#include "login.h"
#include "ui_utils.h"
#include "borrow_ui.h"
#include "log_ui.h"
#include "reader_ui.h"

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

extern void mainMenu(bool isAdmin, int readerId, const std::string& userName, const std::string& roleName);

struct LoginInput {
    int x;
    int y;
    int w;
    int h;
    std::wstring text;
    std::wstring placeholder;
    bool active = false;
    bool password = false;
};

struct LoginRole {
    int x;
    int y;
    int w;
    int h;
    int selected = 0;
};

static IMAGE g_loginBg;
static bool g_hasLoginBg = false;

static void LoadLoginBg() {
    if (std::filesystem::exists("picture/login.png")) {
        loadimage(&g_loginBg, L"picture/login.png", WIN_W, WIN_H);
        g_hasLoginBg = true;
    }
    else {
        g_hasLoginBg = false;
    }
}

static bool CheckAdminLogin(
    const std::wstring& username,
    const std::wstring& password,
    std::string& name,
    std::string& role
) {
    std::ifstream fin("data/admin_accounts.csv");

    if (!fin) {
        return false;
    }

    std::string line;
    std::getline(fin, line);

    std::string inputUser = ws2s(username);
    std::string inputPassword = ws2s(password);

    while (std::getline(fin, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string user;
        std::string pwd;
        std::string realName;
        std::string realRole;

        std::getline(ss, user, ',');
        std::getline(ss, pwd, ',');
        std::getline(ss, realName, ',');
        std::getline(ss, realRole, ',');

        if (user == inputUser && pwd == inputPassword) {
            name = realName.empty() ? user : realName;
            role = realRole.empty() ? "admin" : realRole;
            return true;
        }
    }

    return false;
}

static bool CheckReaderLogin(
    const std::wstring& username,
    const std::wstring& password,
    int& readerId,
    std::string& name
) {
    LoadReaders();
    std::string inputUser = ws2s(username);
    std::string inputPassword = ws2s(password);

    for (const auto& reader : g_readers) {
        if (reader.username == inputUser && reader.password == inputPassword && reader.status != "disabled") {
            readerId = reader.id;
            name = reader.name;
            return true;
        }
    }

    return false;
}

static void DrawInput(const LoginInput& input) {
    setfillcolor(input.active ? RGB(245, 250, 255) : WHITE);
    setlinecolor(input.active ? CLR_PRIMARY : RGB(205, 215, 230));
    solidroundrect(input.x, input.y, input.x + input.w, input.y + input.h, 10, 10);
    roundrect(input.x, input.y, input.x + input.w, input.y + input.h, 10, 10);

    std::wstring show = input.text.empty()
        ? input.placeholder
        : (input.password ? std::wstring(input.text.size(), L'*') : input.text);

    settextcolor(input.text.empty() ? RGB(160, 165, 175) : CLR_TEXT_DARK);
    setbkmode(TRANSPARENT);
    settextstyle_w(20, 0, L"微软雅黑");
    outtextxy_w(input.x + 15, input.y + 13, show);

    if (input.active && clock() % 1000 < 500) {
        int cx = input.x + 15 + textwidth_w(show);
        setlinecolor(CLR_TEXT_DARK);
        line(cx, input.y + 10, cx, input.y + input.h - 10);
    }
}

static void DrawLoginPage(
    const LoginInput& username,
    const LoginInput& password,
    const LoginRole& role,
    const std::wstring& tip
) {
    if (g_hasLoginBg) {
        putimage(0, 0, &g_loginBg);
    }
    else {
        GradientRect(0, 0, WIN_W, WIN_H, RGB(230, 240, 255), RGB(245, 248, 252));
        setbkmode(TRANSPARENT);
        settextcolor(RGB(70, 115, 180));
        settextstyle_w(46, 0, L"微软雅黑");
        outtextxy_w(110, 160, L"Library System");
        settextstyle_w(24, 0, L"微软雅黑");
        outtextxy_w(115, 230, L"图书管理系统课程设计");
    }

    setfillcolor(WHITE);
    setlinecolor(RGB(220, 230, 245));
    solidroundrect(660, 105, 1035, 650, 28, 28);
    roundrect(660, 105, 1035, 650, 28, 28);

    setbkmode(TRANSPARENT);
    settextcolor(RGB(35, 75, 130));
    settextstyle_w(36, 0, L"微软雅黑");
    outtextxy_w(745, 155, L"图书管理系统");

    settextcolor(RGB(135, 150, 170));
    settextstyle_w(18, 0, L"微软雅黑");
    outtextxy_w(765, 205, L"EasyX + CSV Version");

    settextcolor(CLR_TEXT_DARK);
    settextstyle_w(18, 0, L"微软雅黑");
    outtextxy_w(705, 252, L"用户名");
    DrawInput(username);
    outtextxy_w(705, 332, L"密码");
    DrawInput(password);
    outtextxy_w(705, 412, L"身份");

    setfillcolor(WHITE);
    setlinecolor(RGB(205, 215, 230));
    solidroundrect(role.x, role.y, role.x + role.w, role.y + role.h, 10, 10);
    roundrect(role.x, role.y, role.x + role.w, role.y + role.h, 10, 10);

    settextstyle_w(20, 0, L"微软雅黑");
    settextcolor(CLR_TEXT_DARK);
    outtextxy_w(role.x + 15, role.y + 13, role.selected == 0 ? L"管理员" : L"读者");

    settextcolor(CLR_TEXT_LIGHT);
    settextstyle_w(16, 0, L"微软雅黑");
    outtextxy_w(role.x + role.w - 90, role.y + 15, L"点击切换");

    if (!tip.empty()) {
        settextcolor(CLR_DANGER);
        settextstyle_w(17, 0, L"微软雅黑");
        outtextxy_w(705, 493, tip);
    }

    DrawButton(705, 520, 280, 48, L"登录系统");
    DrawButton(705, 585, 125, 38, L"重置");
    DrawButton(860, 585, 125, 38, L"退出");
}

static void ShowOverdueIfNeeded(int readerId) {
    ReloadBorrows();

    int count = 0;
    time_t now = time(nullptr);

    for (const auto& br : g_borrows) {
        if (br.readerId == readerId && br.returnDate == 0 && br.dueDate < now) {
            count++;
        }
    }

    if (count > 0) {
        std::wstring text = L"你有 " + std::to_wstring(count) + L" 本图书已逾期，请尽快归还！";
        MessageBoxW(GetHWnd(), text.c_str(), L"逾期提醒", MB_OK | MB_ICONWARNING);
    }
}

static void TryLogin(LoginInput& username, LoginInput& password, LoginRole& role, std::wstring& tip) {
    if (username.text.empty() || password.text.empty()) {
        tip = L"用户名和密码不能为空";
        return;
    }

    bool isAdmin = role.selected == 0;
    int readerId = -1;
    bool ok = false;
    std::string name;
    std::string roleName;

    if (isAdmin) {
        ok = CheckAdminLogin(username.text, password.text, name, roleName);
    }
    else {
        roleName = "reader";
        ok = CheckReaderLogin(username.text, password.text, readerId, name);
    }

    if (!ok) {
        tip = L"账号、密码或身份选择错误，或账号已停用";
        return;
    }

    AddLog(ws2s(username.text), roleName, "登录", "success");
    MessageBoxW(GetHWnd(), L"登录成功！", L"提示", MB_OK | MB_ICONINFORMATION);

    if (!isAdmin) {
        ShowOverdueIfNeeded(readerId);
    }

    mainMenu(isAdmin, readerId, ws2s(username.text), roleName);
}

void loginMain() {
    EnsureBaseDataFiles();
    EnsureGraphInitialized();
    SetWindowTextW(GetHWnd(), L"登录 - 图书管理系统");
    LoadLoginBg();

    LoginInput username{ 705, 280, 280, 46, L"", L"请输入用户名", false, false };
    LoginInput password{ 705, 360, 280, 46, L"", L"请输入密码", false, true };
    LoginRole role{ 705, 440, 280, 46, 0 };
    std::wstring tip;
    ExMessage msg;

    while (true) {
        while (peekmessage(&msg, EX_MOUSE | EX_CHAR, true)) {
            if (msg.message == WM_LBUTTONDOWN) {
                username.active = PtInRect(msg.x, msg.y, username.x, username.y, username.w, username.h);
                password.active = PtInRect(msg.x, msg.y, password.x, password.y, password.w, password.h);

                if (PtInRect(msg.x, msg.y, role.x, role.y, role.w, role.h)) {
                    role.selected = 1 - role.selected;
                    tip.clear();
                }

                if (PtInRect(msg.x, msg.y, 705, 520, 280, 48)) {
                    TryLogin(username, password, role, tip);
                }

                if (PtInRect(msg.x, msg.y, 705, 585, 125, 38)) {
                    username.text.clear();
                    password.text.clear();
                    tip.clear();
                    username.active = false;
                    password.active = false;
                    role.selected = 0;
                }

                if (PtInRect(msg.x, msg.y, 860, 585, 125, 38)) {
                    closegraph();
                    exit(0);
                }
            }

            if (msg.message == WM_CHAR) {
                LoginInput* activeInput = nullptr;

                if (username.active) {
                    activeInput = &username;
                }

                if (password.active) {
                    activeInput = &password;
                }

                if (activeInput != nullptr) {
                    wchar_t ch = static_cast<wchar_t>(msg.ch);

                    if (ch == L'\b') {
                        if (!activeInput->text.empty()) {
                            activeInput->text.pop_back();
                        }
                    }
                    else if (ch == L'\r') {
                        TryLogin(username, password, role, tip);
                    }
                    else if (ch >= 32) {
                        activeInput->text.push_back(ch);
                    }

                    tip.clear();
                }
            }
        }

        BeginBatchDraw();
        DrawLoginPage(username, password, role, tip);
        EndBatchDraw();
        Sleep(10);
    }
}
