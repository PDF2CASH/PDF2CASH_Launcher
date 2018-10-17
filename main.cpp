#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // Set window in center of the screen.
    w.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            w.size(),
            qApp->desktop()->availableGeometry()
        )
    );

    // Let to show window.
    w.show();

    return a.exec();
}
