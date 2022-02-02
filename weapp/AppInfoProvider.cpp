#include "AppInfoProvider.h"
#include "ApiManager.h"
#include "CustomTimer.h"

#include <QBuffer>
#include <QImageWriter>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QDirIterator>
#include <QDateTime>

AppInfoProvider::AppInfoProvider(QObject *parent)
    : QObject(parent)
    ,  m_isBrowserInfoEnabled(true)
    ,  m_isAppInfoEnabled(true)
{
    m_roundedTimePickerTimer = new QTimer(this);
    m_timerSendInfo = new QTimer(this);
    m_roundedTimePickerTimer->setSingleShot(true);

    connect(m_roundedTimePickerTimer, &QTimer::timeout, this, [this] () {
        onTimeOutSendInfoSlot();
        m_timerSendInfo->start(60000);
    });
    connect(m_timerSendInfo, &QTimer::timeout, this, &AppInfoProvider::onTimeOutSendInfoSlot);
}

void AppInfoProvider::start()
{
    if (m_isAppInfoEnabled || m_isBrowserInfoEnabled) {
        QDateTime startTime = QDateTime::currentDateTime();
        int tillRounded = CustomTimer::getMsecTillTick(startTime.time());
        m_roundedTimePickerTimer->start(tillRounded);
    }
}

void AppInfoProvider::stop()
{
    m_roundedTimePickerTimer->stop();
    m_timerSendInfo->stop();
}

void AppInfoProvider::onTimeOutSendInfoSlot()
{
    std::vector<AppAndBrowserInfo> infoToSend;
    if (m_isAppInfoEnabled) {
        auto appsInfo = getRunningAppInfo();
        infoToSend.insert(infoToSend.end(), appsInfo.begin(), appsInfo.end());
    }
    if (m_isBrowserInfoEnabled) {
        auto browsersInfo = getRunningBrowserInfo();
        infoToSend.insert(infoToSend.end(), browsersInfo.begin(), browsersInfo.end());
    }

    emit sendInfo(infoToSend);
}
