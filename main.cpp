#include "widget.h"
#include "src/db/dbmanager.h"
#include "src/network/serverservice.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!DBManager::instance().initDatabase()) {
        return -1;
    }

    ServerService serverService;
    serverService.start(9500);

    Widget w;
    w.show();
    return a.exec();
}
