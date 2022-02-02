#include "WindowsAppInfoProvider.h"

#include "WindowsBrowserInfoProvider.h"
#include <WinUtilities.h>

#include <winternl.h>
#include <dwmapi.h>

#include <QXmlStreamReader>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QTimer>
#include <QIcon>
#include <QDir>
#include <QDebug>

struct MonitorRects
{
    std::vector<RECT>   rcMonitors;

    static BOOL CALLBACK MonitorEnum(HMONITOR hMon,HDC hdc,LPRECT lprcMonitor,LPARAM pData)
    {
        Q_UNUSED(hMon)
        Q_UNUSED(hdc)
        MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
        pThis->rcMonitors.push_back(*lprcMonitor);
        return TRUE;
    }

    MonitorRects()
    {
        EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
    }
};

WindowsAppInfoProvider::WindowsAppInfoProvider(QObject *parent)
    : AppInfoProvider(parent)
{
    m_browserInfoProvider = new WindowsBrowserInfoProvider(this);
    m_timerForGettingActiveWindow = new QTimer(this);
    m_timerForGettingActiveWindow->setInterval(1000);
    connect(m_timerForGettingActiveWindow, &QTimer::timeout, this, &WindowsAppInfoProvider::gatherInfoOfActiveWindowSlot);
}

void WindowsAppInfoProvider::start()
{
    AppInfoProvider::start();
    if(m_isBrowserInfoEnabled) {
        m_browserInfoProvider->start();
    }
    if(m_isAppInfoEnabled) {
        m_activeWindowMap.clear();
        gatherInfoOfActiveWindowSlot();
        m_timerForGettingActiveWindow->start();
    }
}

void WindowsAppInfoProvider::stop()
{
    AppInfoProvider::stop();
    if(m_isBrowserInfoEnabled) {
        m_browserInfoProvider->stop();
    }
    if(m_isAppInfoEnabled) {
        m_timerForGettingActiveWindow->stop();
    }
}

QString WindowsAppInfoProvider::getScreenIdFromHwnd(HWND hwnd)
{
    HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    GetMonitorInfo(hmon, &mi);

    MonitorRects screens;
    for(size_t i = 0; i < screens.rcMonitors.size(); i++){
        if(screens.rcMonitors.at(i).left == mi.rcMonitor.left &&
                screens.rcMonitors.at(i).top == mi.rcMonitor.top &&
                screens.rcMonitors.at(i).right == mi.rcMonitor.right &&
                screens.rcMonitors.at(i).bottom == mi.rcMonitor.bottom){
            return QString::number(i);
        }
    }
    return "0";
}

std::vector<AppAndBrowserInfo> WindowsAppInfoProvider::getRunningAppInfo()
{
    addTimeDiffToLastActiveWindow();

    std::vector<AppAndBrowserInfo> appInfos;
    for(auto runningAppInfo : m_activeWindowMap) {
        AppAndBrowserInfo appInfo;
        QString name = runningAppInfo.second.path.split("/").last();
        if(runningAppInfo.second.path.contains("C:/Windows/SystemApps")){
            continue;
        }
        if(name == "explorer.exe" || name == "ApplicationFrameHost.exe"){
            continue;
        }
        appInfo.name = name.left(name.count() - 4);

        QPixmap icon = QPixmap();
        if(runningAppInfo.second.path.contains("C:/Program Files/WindowsApps")){
            QString iconPath = runningAppInfo.second.path.replace(name, "");
            iconPath =  getAppIconPathFromFolder(iconPath.left(iconPath.length() - 1));
            icon = QPixmap(iconPath);
            if(icon.isNull()){
                icon = QPixmap(":/icon/executable.svg");
            }
            else {
                icon = icon.scaled(64, 64, Qt::AspectRatioMode::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            }
        }

        if(icon.isNull())
            icon = getIconToSize(getFileIcon(runningAppInfo.second.path), 64);

        appInfo.icon = icon;
        appInfo.type = "APP";
        appInfo.screenId = getScreenIdFromHwnd(runningAppInfo.second.hwnd);
        appInfo.runDuration = (double)runningAppInfo.second.runTime / 1000;
        appInfos.push_back(appInfo);
    }
    m_activeWindowMap.clear();
    return appInfos;
}

std::vector<AppAndBrowserInfo> WindowsAppInfoProvider::getRunningBrowserInfo()
{
    return m_browserInfoProvider->getBrowsersInfo();
}

QString WindowsAppInfoProvider::getAppIconPathFromFolder(const QString &folderPath)
{
    QString appPath = folderPath;
    QString manifestFilePath = folderPath + "/AppxManifest.xml";
    QFile xmlFile(manifestFilePath);
    if (!xmlFile.open(QFile::ReadOnly)) {
        QStringList list = folderPath.split("/");
        list.removeLast();
        appPath = list.join("/");
        manifestFilePath = appPath + "/AppxManifest.xml";
        xmlFile.setFileName(manifestFilePath);
        if (!xmlFile.open(QFile::ReadOnly)){
            qDebug() << "[WindowsAppIconProvider] can not open app manifest file: " + manifestFilePath;
            return QString();
        }
    }

    QXmlStreamReader reader(&xmlFile);

    while (reader.readNext() != QXmlStreamReader::Invalid) {
        if (reader.name() == "Properties") {
            while (reader.readNext() != QXmlStreamReader::Invalid) {
                if (reader.name() == "Logo") {
                    QString localPath = reader.readElementText();
                    localPath.replace('\\', '/');
                    return getAppIconPath(appPath, localPath);
                }
            }
        }
    }
    return QString();
}

QString WindowsAppInfoProvider::getAppIconPath(const QString &folderPath, const QString &iconLocalPath)
{
    QString iconLocalPathWithoutExtension = QString(iconLocalPath).remove(".png");
    QStringList splitted = iconLocalPathWithoutExtension.split('/');
    QString iconAbsoluteFolderPath = folderPath;
    for (int i = 0; i < splitted.size() - 1; ++i) {
        iconAbsoluteFolderPath += "/" + splitted.at(i);
    }
    QString iconFileName = splitted.back();

    QDir dir(iconAbsoluteFolderPath);
    QString path = findIconInDir(dir, iconFileName);

    return path;
}

QString WindowsAppInfoProvider::findIconInDir(const QDir &dir, const QString &iconFileName)
{
    QStringList folderList = dir.entryList();
    QRegExp regExp(iconFileName + ".*"  + ".*.png");
    int index = folderList.indexOf(regExp);

    if (index == -1) {
        regExp = QRegExp(iconFileName + ".*.png");
        index = folderList.indexOf(regExp);
    }

    if (index != -1) {
        return dir.absoluteFilePath(folderList.at(index));
    }

    return QString();
}

QIcon WindowsAppInfoProvider::getFileIcon(const QString &path)
{
    QIcon icon;
    QFileIconProvider iconProvider;
    icon =  iconProvider.icon(QFileInfo(path));
    return icon;
}

QPixmap WindowsAppInfoProvider::getIconToSize(const QIcon &icon, int desiredSize)
{
    int sizeDiff = 0, index = -1;
    const QList<QSize> &availableSizes = icon.availableSizes();
    for(int i = 0, length = availableSizes.size(); i < length; i++){
        const QSize& currentSize = availableSizes.at(i);
        int currentDiff = qAbs(qMax(currentSize.width(), currentSize.height()) - desiredSize);
        if(currentDiff < sizeDiff || index == -1){
            index = i;
            sizeDiff = currentDiff;
        }
    }
    if(index == -1){
        //no match
        return QPixmap(":/icon/executable.svg");;
    }else if(sizeDiff != 0){
        //scale pixmap
        QPixmap pixmap = icon.pixmap(availableSizes.at(index));
        if(pixmap.width() >= pixmap.height()){
            return pixmap.scaledToWidth(desiredSize, Qt::SmoothTransformation);
        }else{
            return pixmap.scaledToHeight(desiredSize, Qt::SmoothTransformation);
        }
    }else{
        //perfect match
        return icon.pixmap(availableSizes.at(index));
    }
}


struct EnumWindowParams
{
    WindowsAppInfoProvider* controller;
};

BOOL CALLBACK EnumWindowsProc(HWND wnd, LPARAM lParam)
{
    EnumWindowParams params =  *reinterpret_cast<EnumWindowParams*>(lParam);
    return params.controller->enumerateWindowsCallback(wnd);
}

BOOL CALLBACK WindowsAppInfoProvider::enumerateWindowsCallback(HWND hwnd)
{
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);

    QString processExe = getProcessExeNameByID(processId);

    processExe.replace('\\', '/');
    if(!processExe.contains("C:/Program Files/WindowsApps")){
        if (!IsWindowVisible(hwnd)) {
            return TRUE;
        }
    }
    QString skypeBridge = processExe.split("/").last();
    if(skypeBridge == "SkypeBridge.exe"){
        return TRUE;
    }
    RunningAppInfo info;
    info.path = processExe;
    info.hwnd = hwnd;
    m_runningAppsInfo.push_back(info);

    return TRUE;
}
//Unused function
std::vector<RunningAppInfo> WindowsAppInfoProvider::getPathsOfRunningApps()
{
    EnumWindowParams params;
    params.controller = this;

    EnumWindows(EnumWindowsProc, (LPARAM)(&params));    //synchronous

    std::sort(m_runningAppsInfo.begin(), m_runningAppsInfo.end());
    auto it = std::unique(m_runningAppsInfo.begin(), m_runningAppsInfo.end());
    m_runningAppsInfo.erase(it, m_runningAppsInfo.end());

    return m_runningAppsInfo;
}

void WindowsAppInfoProvider::gatherInfoOfActiveWindowSlot()
{
    HWND hwnd=GetForegroundWindow();
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);

    QString processExe = getProcessExeNameByID(processId);
    if(processExe.isEmpty()) return;
    RunningAppInfo info;
    info.path = processExe;
    info.hwnd = hwnd;

    if(!m_activeWindowMap.empty()){
        if(processExe != m_lastActiveWindow){
            addTimeDiffToLastActiveWindow();
            if(m_activeWindowMap.count(processExe) == 0)
                m_activeWindowMap[processExe] = info;
        }
    }else{
        m_startTimeOfActiveWindow = QDateTime::currentDateTime().toUTC();
        m_activeWindowMap[processExe] = info;
    }
    m_lastActiveWindow = processExe;
}

void WindowsAppInfoProvider::addTimeDiffToLastActiveWindow()
{
    QDateTime now = QDateTime::currentDateTime().toUTC();
    m_activeWindowMap[m_lastActiveWindow].runTime += m_startTimeOfActiveWindow.msecsTo(now);
    m_startTimeOfActiveWindow = now;
}
