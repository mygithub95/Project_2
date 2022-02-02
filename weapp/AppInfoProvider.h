#ifndef APPINFOPROVIDER_H
#define APPINFOPROVIDER_H

#include <QObject>
#include "Info.h"
#include <QPixmap>
#include <QMap>

class QTimer;
struct RunningAppInfo;

class AppInfoProvider : public QObject
{
    Q_OBJECT
public:
    explicit AppInfoProvider(QObject *parent = nullptr);

    virtual void start();
    virtual void stop();

    void setBrowserInfoEnabled(bool browserInfoEnabled) {m_isBrowserInfoEnabled = browserInfoEnabled;}
    void setAppInfoEnabled(bool appInfoEnabled) {m_isAppInfoEnabled = appInfoEnabled;}

protected:
    virtual std::vector<AppAndBrowserInfo> getRunningAppInfo() = 0;
    virtual std::vector<AppAndBrowserInfo> getRunningBrowserInfo() = 0;

protected:
    QTimer *m_timerSendInfo;
    bool m_isBrowserInfoEnabled;
    bool m_isAppInfoEnabled;
    QTimer *m_roundedTimePickerTimer;

signals:
    void sendInfo(std::vector<AppAndBrowserInfo>);

private slots:
    void onTimeOutSendInfoSlot();
};

#endif // APPINFOPROVIDER_H
