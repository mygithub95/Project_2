#ifndef SCREENRECORDERTHREAD_H
#define SCREENRECORDERTHREAD_H

#include <QThread>
#include "Info.h"

class VideoCreator;
class QTimer;

class ScreenRecorderThread : public QThread
{
    Q_OBJECT

public:
    ScreenRecorderThread(int screenId, int second, int frameCountPerSec);

    void stop();

protected:
    void run() override;

private:
    QTimer *m_timer;
    int m_screenId;
    int m_second;
    int m_frameCountPerSec;
    VideoCreator* m_videoCreator;

signals:
    void screenRecordReady(VideoFilesInfo info);
    void stopTimer(bool isStopped);
};

#endif // SCREENRECORDERTHREAD_H
