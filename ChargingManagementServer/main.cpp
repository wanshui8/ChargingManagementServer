#include "widget.h"
#include "src/db/dbmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //连接数据库
    if (!DBManager::instance().initDatabase()) {
            return -1;   // 连接失败则退出
        }

    Widget w;
    w.show();
    return a.exec();
}
