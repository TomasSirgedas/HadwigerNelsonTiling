#include "HadwigerNelsonTiling.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HadwigerNelsonTiling w;
    w.show();
    return a.exec();
}
