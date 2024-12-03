#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // a.setWindowIcon(QIcon("appicon.ico"));
    QDir::setCurrent(a.applicationDirPath());
    MainWindow w;
    w.show();
    return a.exec();
}
