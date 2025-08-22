#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // THÊM MỚI: Thiết lập thông tin cho QSettings
    QCoreApplication::setOrganizationName("MyCompany");
    QCoreApplication::setApplicationName("FrameCapture");

    MainWindow w;
    w.show();
    return a.exec();
}
