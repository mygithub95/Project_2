#ifndef WINDOWSBROWSERINFOPROVIDER_H
#define WINDOWSBROWSERINFOPROVIDER_H

#ifdef _WIN32

#include <QObject>

#include "Info.h"

class QTimer;


class WindowsBrowserInfoProvider : public QObject
{
    Q_OBJECT
public:
    explicit WindowsBrowserInfoProvider(QObject *parent = nullptr);
    void start();
    void stop();
    std::vector<AppAndBrowserInfo> getBrowsersInfo();

private:
    enum BrowserType{
        eChrome,  eMozilla, eMicrosoftEdge
    };

    void creatNewBrowserInfo(const std::pair<const QString, QString> tabinfo);
    QString getNameFromURL(const QString &url);
    std::map<QString, QString> getActiveTabs(BrowserType type);

    std::map<QString, AppAndBrowserInfo> m_browsersInfo;
    std::map<QString, QString> m_activeTabs;
    QTimer *m_timerForDownloadBrowserIcon;

private slots:
    void onTimerOutDownloadIconSlot();

};

#endif

#endif // WINDOWSBROWSERINFOPROVIDER_H
