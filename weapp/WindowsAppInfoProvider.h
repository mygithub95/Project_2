#ifndef WINDOWSAPPINFOPROVIDER_H
#define WINDOWSAPPINFOPROVIDER_H

#include "AppInfoProvider.h"

#include <QStringList>
#include <QRect>
#include <QDateTime>

#include <windows.h>


class QDir;
class WindowsBrowserInfoProvider;

struct RunningAppInfo{
    RunningAppInfo()
        : runTime(0)
    {}
    int runTime;
    QString path;
    HWND hwnd;

    bool operator<(const RunningAppInfo& other) const
    {
        return (this->path < other.path);
    }
    bool operator==(const RunningAppInfo &other)
    {
        return (this->path == other.path);
    }
};

class WindowsAppInfoProvider : public AppInfoProvider
{
public:
    WindowsAppInfoProvider(QObject *parent = nullptr);

    void start() override;
    void stop() override;

    BOOL CALLBACK enumerateWindowsCallback(HWND hwnd);

private:
    QString getScreenIdFromHwnd(HWND hwnd);

    std::vector<RunningAppInfo> getPathsOfRunningApps();
    std::vector<AppAndBrowserInfo> getRunningAppInfo() override;
    std::vector<AppAndBrowserInfo> getRunningBrowserInfo() override;

    QString getAppIconPathFromFolder(const QString& folderPath);
    QString getAppIconPath(const QString& folderPath, const QString& iconLocalPath);
    QString findIconInDir(const QDir& dir, const QString& iconFileName);

    QIcon getFileIcon(const QString &path);
    QPixmap getIconToSize(const QIcon &icon, int desiredSize);

    void addTimeDiffToLastActiveWindow();

private:
    std::vector<RunningAppInfo> m_runningAppsInfo;
    std::map<QString, RunningAppInfo> m_activeWindowMap;
    WindowsBrowserInfoProvider *m_browserInfoProvider;

    QTimer* m_timerForGettingActiveWindow;
    QDateTime m_startTimeOfActiveWindow;
    QString m_lastActiveWindow;

private slots:
    void gatherInfoOfActiveWindowSlot();
};

#endif // WINDOWSAPPINFOPROVIDER_H
