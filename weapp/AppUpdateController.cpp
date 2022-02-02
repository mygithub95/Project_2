#include "AppUpdateController.h"
#include "Logger.h"

#include <QTimer>
#include <QtXml>
#include <QString>
#include <QApplication>
#ifdef _WIN32
#include <QtWinExtras>
#endif
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#define UPDATES_CHECK_INTERVAL 3600 //sec

AppUpdateController::AppUpdateController(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(UPDATES_CHECK_INTERVAL *1000);
    connect(m_timer, &QTimer::timeout, this, &AppUpdateController::checkForUpdates);
}

void AppUpdateController::checkForUpdates()
{
    QFutureWatcher<void>* futureWatcher = new QFutureWatcher<void>;
    futureWatcher->setFuture(QtConcurrent::run(this, &AppUpdateController::runToolForCheckingUpdates));
    connect(futureWatcher, &QFutureWatcher<void>::finished, this, &AppUpdateController::onCheckerThreadFinished);
    if(!m_timer->isActive())
        m_timer->start();
}

void AppUpdateController::onCheckerThreadFinished()
{
    dynamic_cast<QFutureWatcher<void> *>(QObject::sender())->deleteLater();
    QString version = "";
    if(!m_data.isEmpty()){
        QDomDocument document;
        if(document.setContent(m_data)){
            QDomNodeList list = document.elementsByTagName("update");
            version = list.at(0).toElement().attribute("version");
        }
    }
    emit updateIsAvailable(version);
}

void AppUpdateController::updateApplicationSlot()
{
    //launch updater, then immediately shutdown
#ifdef _WIN32
    std::wstring lpOperation =  QString("runas").toStdWString();
    std::wstring lpFile = QString(QApplication::applicationDirPath() + "/maintenancetool.exe").toStdWString();
    std::wstring lpParameters = QString("--updater --script " + QApplication::applicationDirPath() + "/auto_updater.qs").toStdWString();
    std::wstring lpDirectory = QApplication::applicationDirPath().toStdWString();
    HINSTANCE result = ShellExecute(NULL, lpOperation.c_str(), lpFile.c_str(), lpParameters.c_str(), lpDirectory.c_str(), SW_SHOWDEFAULT);
    if(reinterpret_cast<intptr_t>(result) > 32){    //according to the documentation, this is a correct way to check success
        QString logMessage = QString("Update initiation successful, result is: %1").arg(reinterpret_cast<intptr_t>(result));
        qDebug()<<logMessage;
        Logger::instance()->log(logMessage);
        QApplication::exit();
    }else{
        QString logMessage = QString("Update initiation unsuccessful, result is: %1").arg(reinterpret_cast<intptr_t>(result));
        qDebug()<<logMessage;
        Logger::instance()->log(logMessage);
    }
#elif __linux__
    QProcess process;
    QString program = QApplication::applicationDirPath() + "/maintenancetool";
    QStringList arguments;
    arguments << "--updater"<<"--script"<<QApplication::applicationDirPath() + "/auto_updater.qs";
    process.startDetached(program, arguments);
    process.waitForStarted();
    QApplication::exit();
#elif __APPLE__
    QString path = QApplication::applicationDirPath();
    int pos = path.lastIndexOf("WEAPP.app");
    QString program = path.left(pos) + "maintenancetool.app/Contents/MacOS/maintenancetool";
    QProcess process;
    QStringList arguments;
    arguments << "--updater"<<"--script"<<path.left(pos) + "auto_updater.qs";
    process.startDetached(program, arguments);
    process.waitForStarted();
    QApplication::exit();
#endif
    //updater will start the application after update is finished
}

void AppUpdateController::runToolForCheckingUpdates()
{
    QProcess process;
    Logger::instance()->log("Maintenancetool path is: " + QApplication::applicationDirPath());
#ifdef _WIN32
    process.start(QApplication::applicationDirPath() + "/maintenancetool.exe --checkupdates");
#elif __linux__
    process.start(QApplication::applicationDirPath()+"/maintenancetool --checkupdates");
#elif __APPLE__
    QString path = QApplication::applicationDirPath();
    int pos = path.lastIndexOf("WEAPP.app");
    path = path.left(pos) + "maintenancetool.app/Contents/MacOS";
    process.start(path + "/maintenancetool --checkupdates");
#endif
    process.waitForFinished();
    if(process.error() != QProcess::UnknownError)
    {
        //error checking for updates
        Logger::instance()->log("Error checking for updates.");
        return;
    }
    m_data = process.readAllStandardOutput();
    Logger::instance()->log("Available update data is: "+ m_data);
}
