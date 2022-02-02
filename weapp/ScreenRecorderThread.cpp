#include "ScreenRecorderThread.h"
#include "CreateVideoFromScreenshots.h"

#include <QTimer>

ScreenRecorderThread::ScreenRecorderThread(int screenId, int second, int frameCountPerSec)
    : m_screenId(screenId)
    , m_second(second)
    , m_frameCountPerSec(frameCountPerSec)
{
    connect(this, &QThread::finished, this, &QObject::deleteLater);
}

void ScreenRecorderThread::stop()
{
   emit stopTimer(true);
}

void ScreenRecorderThread::run()
{
    if(!m_second || !m_frameCountPerSec)
        return;
    m_timer = new QTimer(nullptr);
    m_videoCreator = new VideoCreator(m_screenId, m_second, m_frameCountPerSec);
    connect(m_timer, &QTimer::timeout, m_videoCreator, &VideoCreator::write);

    connect(this, &ScreenRecorderThread::stopTimer, m_timer, &QTimer::stop, Qt::QueuedConnection);
    connect(this, &ScreenRecorderThread::stopTimer, m_videoCreator, &VideoCreator::end, Qt::QueuedConnection);
    qRegisterMetaType<VideoFilesInfo>("VideoFilesInfo");
    connect(m_videoCreator, &VideoCreator::gifPrepared, this, [this](bool isStopped, VideoFilesInfo info){
       m_timer->stop();
       if(!isStopped)
           emit screenRecordReady(info);
       quit();
    }, Qt::DirectConnection);
    connect(this, &QThread::finished, m_videoCreator, &QObject::deleteLater);
    connect(this, &QThread::finished, m_timer, &QObject::deleteLater);

    //m_timer->start(1000 / m_frameCountPerSec);
    m_timer->start(1000);
    QThread::run();
}
