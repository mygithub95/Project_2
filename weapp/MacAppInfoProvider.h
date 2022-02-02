#ifndef MACAPPINFOPROVIDER_H
#define MACAPPINFOPROVIDER_H

#include "AppInfoProvider.h"

#include <set>
#include <QDateTime>

struct RunningAppInfo
{

};

class MacAppInfoProvider : public AppInfoProvider
{
public:
    explicit MacAppInfoProvider(QObject *parent = nullptr);
    void start() override;
    void stop() override;
protected:
    std::vector<AppAndBrowserInfo> getRunningAppInfo() override;
    std::vector<AppAndBrowserInfo> getRunningBrowserInfo() override;

private slots:
    void onBrowserInfoTimerTimeOutSlot();
    void gatherInfoOfActiveWindowSlot();
private:
    void updateBrowsersMainInfo();
    void updateBrowsersInfoIcons();
    QString getNameFromURL(const QString& url);
    void onURLIconDownloadedSlot(QPixmap);

    void addTimeDiffToLastActiveWindow();

    QTimer* m_browserInfoTimer;
    std::vector<AppAndBrowserInfo> m_browsersInfo;
    std::vector<QString> m_browsersInfoURLs;

    std::map<QString, AppAndBrowserInfo> m_activeWindowMap;

    QTimer* m_timerForGettingActiveWindow;
    QDateTime m_startTimeOfActiveWindow;
    QString m_lastActiveWindow;
};

#endif // MACAPPINFOPROVIDER_H
