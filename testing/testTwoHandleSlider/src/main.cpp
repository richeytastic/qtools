#include <QApplication>
#include "MainWindow.h"


int main( int argc, char* argv[])
{
    Q_INIT_RESOURCE(res);
    QApplication qapp( argc, argv);

    MainWindow w;
    w.show();

    return qapp.exec();
}   // end main
