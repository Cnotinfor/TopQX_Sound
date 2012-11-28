#include <QtGui/QApplication>
#include "MusicExample.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MusicExample w;
    w.show();

    return a.exec();
}
