#include "login.h"
#include "ui_utils.h"
#include <map>

static std::map<std::string, std::string> adminMap;

static void LoadAdmins() {
    std::ifstream fin("data/admin_accounts.csv");
    if (!fin) return;
    std::string line;
    std::getline(fin, line);
    while (std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string u, p;
        std::getline(ss, u, ',');
        std::getline(ss, p, ',');
        adminMap[u] = p;
    }
}

static void SaveAdmin(const std::string& u, const std::string& p) {
    adminMap[u] = p;
    std::ofstream fout("data/admin_accounts.csv");
    fout << "username,password\n";
    for (const auto& kv : adminMap) fout << kv.first << "," << kv.second << '\n';
}

void loginMain() {
    EnsureGraphInitialized();
    // 加载右上角校徽
    IMAGE logoImg;
    IMAGE logoSmall;
    bool hasLogo = false;
    int logoW = 0, logoH = 0;
    std::ifstream lf("picture/logoImg.jpg");
    if (lf) {
        lf.close();
        // 先加载原图获取尺寸
        loadimage(&logoImg, L"picture/logoImg.jpg");
        logoW = logoImg.getwidth();
        logoH = logoImg.getheight();
        // 按原尺寸的 1/3 重新加载缩放图
        int sw = logoW / 2; if (sw < 1) sw = 1;
        int sh = logoH / 2; if (sh < 1) sh = 1;
        loadimage(&logoSmall, L"picture/logoImg.jpg", sw, sh);
        hasLogo = true;
    }
    SetWindowTextW(GetHWnd(), L"图书馆系统 - 登录");
    LoadAdmins();
    std::wstring user, pwd;
    int active = 0; // 0-username,1-password
    std::vector<int> cursors(2, 0);
    int mode = 0; // 0-login, 1-register
    std::wstring msg;
    ExMessage m;

    while (true) {
        BeginBatchDraw();
        setbkcolor(CLR_BG);
        cleardevice();
        settextstyle(40, 0, L"微软雅黑");
        settextcolor(CLR_PRIMARY);
        outtextxy(380, 80, L"图书管理系统");
        settextstyle(20, 0, L"微软雅黑");
        settextcolor(CLR_TEXT_DARK);
        outtextxy(350, 180, L"用户名:");
        DrawInputBox(450, 175, 260, 40, user, active == 0, cursors[0]);
        outtextxy(350, 250, L"密  码:");
        DrawInputBox(450, 245, 260, 40, pwd, active == 1, cursors[1]);
        DrawButton(420, 340, 120, 45, mode == 0 ? L"登录" : L"注册");
        DrawButton(560, 340, 120, 45, L"切换注册");
        if (!msg.empty()) {
            settextcolor(RED);
            outtextxy(450, 420, msg.c_str());
        }
        // 绘制右上角校徽（如果加载成功）
        if (hasLogo) {
            int sw = logoSmall.getwidth();
            int sh = logoSmall.getheight();
            putimage(WIN_W - sw - 20, 20, &logoSmall);
        }
        EndBatchDraw();

        m = getmessage(EX_CHAR | EX_KEY | EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x, y = m.y;
            if (PtInRect(x, y, 450, 175, 260, 40)) active = 0;
            else if (PtInRect(x, y, 450, 245, 260, 40)) active = 1;
            else if (PtInRect(x, y, 420, 340, 120, 45)) {
                std::string u = ws2s(user), p = ws2s(pwd);
                if (mode == 0) {
                    if (adminMap.count(u) && adminMap[u] == p) {
                        extern void mainMenu();
                        mainMenu();
                        return;
                    }
                    else msg = L"用户名或密码错误";
                }
                else {
                    if (adminMap.count(u)) msg = L"用户名已存在";
                    else { SaveAdmin(u, p); msg = L"注册成功，请登录"; mode = 0; }
                }
            }
            else if (PtInRect(x, y, 560, 340, 120, 45)) {
                mode = !mode; msg.clear(); user.clear(); pwd.clear(); cursors = { 0,0 };
            }
        }
        else if (m.message == WM_CHAR && active >= 0) {
            wchar_t ch = m.ch;
            auto& buf = (active == 0) ? user : pwd;
            auto& cp = cursors[active];
            if (ch == '\b') { if (!buf.empty()) { buf.pop_back(); if (cp) --cp; } }
            else if (ch >= 32) { buf += ch; ++cp; }
        }
    }
}