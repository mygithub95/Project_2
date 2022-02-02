#include "LaunchAppController.h"
#include  "Logger.h"

#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDebug>

LaunchAppController::LaunchAppController()
{

}

void LaunchAppController::addAppOnStartup()
{
    removeAppFromStartup();
#ifdef _WIN32
     QFile file;
     file.setFileName(qApp->applicationFilePath());
     bool link = file.link(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup/WEAPP.lnk");
     Logger::instance()->log("LaunchAppController::addAppOnStartup: Link file created is " + QString::number(link));
#elif __APPLE__
    QDir autostartDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Library/LaunchAgents";
    if(!autostartDir.exists())
        autostartDir.mkpath(".");
    QString filePath = autostartDir.path() + "/WEAPP.plist";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        Logger::instance()->log("LaunchAppController::addAppOnStartup: Could not open plist file.");
        return;
    }

    QString content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
<plist version=\"1.0\">\n\
<dict>\n\
    <key>Label</key>\n\
    <string>WEAPP</string>\n\
    <key>ProgramArguments</key>\n\
    <array>\n\
        <string>" + qApp->applicationFilePath() + "</string>\n\
    </array>\n\
    <key>ProcessType</key>\n\
    <string>Interactive</string>\n\
    <key>RunAtLoad</key>\n\
    <true/>\n\
    <key>KeepAlive</key>\n\
    <false/>\n\
</dict>\n\
</plist>";

    QTextStream out(&file);
    out << content;
    file.close();

#elif __linux__

    QDir autostartDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart");
    if(!autostartDir.exists())
        autostartDir.mkpath(".");
    Logger::instance()->log("LaunchAppController::addAppOnStartup: autostartDir == " + autostartDir.path());
    QString filePath = autostartDir.path() + "/WEAPP.desktop";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        Logger::instance()->log("LaunchAppController::addAppOnStartup: Could not open WEAPP.desktop file.");
        return;
    }

    QString content =
    "[Desktop Entry]\n\
    Version=1.0\n\
    Name=WEAPP\n\
    Comment=Running WEAPP on startup\n\
    Exec=bash " + qApp->applicationDirPath() + "/WEAPP_script.sh\n\
    Icon="+ qApp->applicationDirPath() + "/WEAPP_icon.png\n\
    Terminal=false\n\
    Type=Application\n\
    Categories=Application;\n";

    QTextStream out(&file);
    out << content;
    file.close();
#endif
}

void LaunchAppController::removeAppFromStartup()
{
     QString filePath;
#ifdef _WIN32
    filePath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/Startup/WEAPP.lnk";
#elif __APPLE__
    QString dirName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Library/LaunchAgents";
    filePath = dirName + "/WEAPP.plist";
#elif __linux__
    filePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/WEAPP.desktop";
#endif
     if(QFile::exists(filePath)){
         bool remove = QFile::remove(filePath);
         Logger::instance()->log("LaunchAppController::removeAppFromStartup: "
                                 "file removed from startup " + QString::number(remove));
     }
}
