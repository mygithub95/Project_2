#include "MainWindow.h"

#include <QApplication>
#ifndef __APPLE__
#include <QWebEngineProfile>
#else
#include "MacDockHandler.h"
#endif

#include <QDir>
#include <QLockFile>
#include <QMessageBox>
#include <QIcon>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("TimeTracking");
    QCoreApplication::setApplicationName("WEAPP");
#ifdef _WIN32
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
#endif
    QApplication a(argc, argv);
#ifndef __APPLE__
    QWebEngineProfile::defaultProfile()->clearHttpCache();
#endif

    QLockFile lockFile(QDir::tempPath() + "/WEAPP.lock");
    if(!lockFile.tryLock()){
        QMessageBox msg;
        msg.setText("The application is already running.");
        msg.exec();
        return 0;
    }
    a.setQuitOnLastWindowClosed(false);
    a.setWindowIcon(QIcon(QApplication::applicationDirPath() + "/main-icon.ico"));
    MainWindow w;
#ifdef __APPLE__
    a.installEventFilter(w.getAppTrayIconHandler());
#endif
    w.show();

    return a.exec();
}
