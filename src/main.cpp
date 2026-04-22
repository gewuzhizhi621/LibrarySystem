#include <graphics.h>
#include "login.h"
#include "ui_utils.h"
#include "book_ui.h"
#include "borrow_ui.h"
#include "reader_ui.h"
#include "stat_ui.h"

void mainMenu() {
    EnsureGraphInitialized();
    SetWindowTextW(GetHWnd(), L"图书管理系统 - 主菜单");
    IMAGE bg;
    loadimage(&bg, L"picture/bgImg.png", WIN_W, WIN_H);
    ExMessage m;
    while (true) {
        BeginBatchDraw();
        putimage(0, 0, &bg);
        settextstyle(44, 0, L"微软雅黑");
        settextcolor(WHITE);
        outtextxy(400, 60, L"智慧图书馆");
        DrawButton(400, 180, 300, 60, L"📚 图书管理");
        DrawButton(400, 270, 300, 60, L"📖 借阅管理");
        DrawButton(400, 360, 300, 60, L"👥 读者管理");
        DrawButton(400, 450, 300, 60, L"📊 统计分析");
        DrawButton(400, 540, 300, 60, L"🚪 退出系统");
        EndBatchDraw();

        m = getmessage(EX_MOUSE);
        if (m.message == WM_LBUTTONDOWN) {
            int x = m.x, y = m.y;
            if (PtInRect(x, y, 400, 180, 300, 60)) { bookUIMain(); }
            else if (PtInRect(x, y, 400, 270, 300, 60)) { borrowUIMain(); }
            else if (PtInRect(x, y, 400, 360, 300, 60)) { readerUIMain(); }
            else if (PtInRect(x, y, 400, 450, 300, 60)) { statUIMain(); }
            else if (PtInRect(x, y, 400, 540, 300, 60)) exit(0);
        }
    }
}

int main() {
    loginMain();
    return 0;
}