#include <QApplication>
#include "GameApplication.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GameApplication gameApp;
    gameApp.initialize();

    return app.exec();
}
