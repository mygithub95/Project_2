#include "CustomTimer.h"
#include "Logger.h"

#include <QTimer>
#include <QDebug>
#include <QDateTime>

#include <ctime>

CustomTimer::CustomTimer(QObject *parent)
    : QObject(parent)
    , m_isEnabled(true)
{
    srand(time(0));
    m_tickTimer = new QTimer(this);
    m_tickTimer->setSingleShot(true);
    connect(m_tickTimer, &QTimer::timeout, this, [this] () { onTickTimerTimeoutSlot(true); });
}


int CustomTimer::getMsecTillTick(const QTime& nowTime)
{
    int leftsec = 60 - nowTime.second();
    int leftMsec =  leftsec * 1000 - nowTime.msec();
    return leftMsec;
}

void CustomTimer::onTickTimerTimeoutSlot(bool useThisTick)
{
    QDateTime now = QDateTime::currentDateTime();
    if (useThisTick) {
        emit startScreenRecord(now.toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    }
    else{
        int timeLeft = getMsecTillTick(now.time());
        qDebug() << "time left: " << timeLeft;
        Logger::instance()->log("time left: " + QString::number(timeLeft));
        m_tickTimer->start(timeLeft + 1000);
    }
}

void CustomTimer::start()
{
    if(m_isEnabled){
       onTickTimerTimeoutSlot(false);
    }
}

void CustomTimer::stop()
{
    m_tickTimer->stop();
}

void CustomTimer::setTimerEnabled(bool enabled)
{
    m_isEnabled = enabled;
}

