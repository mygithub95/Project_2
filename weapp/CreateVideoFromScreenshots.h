#ifndef CREATEVIDEOFROMSCREENSHOTS_H
#define CREATEVIDEOFROMSCREENSHOTS_H

#include <QObject>
#include <QDateTime>
#include "Info.h"

#include "FFMPEGVideoWriter.h"

class QTimer;

class VideoCreator : public QObject
{
    Q_OBJECT

public:
    VideoCreator(int screenId, int second, int frameCountPerSec);
    ~VideoCreator();

    void end(bool isSopped = false);

private:
    QString m_fileName;
    QString m_miniVideoName;
    int m_screenId;
    int m_second;
    int m_frameCountPerSec;
    int m_frameNumber;
    int m_timeTillTick;
    QDateTime m_startTime;
    FFMPEGVideoWriter* m_videoWriter;
    FFMPEGVideoWriter* m_miniVideoWriter;

    int m_imageHeight;
    int m_miniVideoHeight;

    uchar* getAlignedImgForFFMPEG(QImage& img, FFMPEGVideoWriter* videoWriter);

signals:
    void gifPrepared(bool isStopped, VideoFilesInfo info);

public slots:
    void write();

};

#endif // CREATEVIDEOFROMSCREENSHOTS_H
