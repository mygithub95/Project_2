#include "CreateVideoFromScreenshots.h"
#include "CustomTimer.h"

#include <QApplication>
#include <QScreen>
#include <QImage>
#include <QPixmap>
#include <QDir>

#include <QThread>

#define IMAGE_WIDTH 1280
#define MINI_VIDEO_WIDTH 256

VideoCreator::VideoCreator(int screenId, int second, int frameCountPerSec)
    : m_screenId(screenId)
    , m_second(second)
    , m_frameCountPerSec(frameCountPerSec)
    , m_frameNumber(0)
{
    m_fileName = QDir::tempPath() + "/TimeTracker/screenVideo_" + QString::number(screenId) + ".mp4";
    m_miniVideoName = QDir::tempPath() + "/TimeTracker/screenMiniVideo_" + QString::number(screenId) + ".mp4";
    m_videoWriter = new FFMPEGVideoWriter();
    m_miniVideoWriter = new FFMPEGVideoWriter();
    auto screens = QApplication::screens();
    m_startTime = QDateTime::currentDateTime();
    m_timeTillTick = CustomTimer::getMsecTillTick(m_startTime.time());
    m_imageHeight = IMAGE_WIDTH * screens[m_screenId]->geometry().height() / screens[m_screenId]->geometry().width();
    m_miniVideoHeight = MINI_VIDEO_WIDTH * screens[m_screenId]->geometry().height() / screens[m_screenId]->geometry().width();
    if(m_imageHeight % 2 != 0)
        m_imageHeight++;
    if(m_miniVideoHeight % 2 != 0)
        m_miniVideoHeight++;
    m_videoWriter->setup(m_fileName.toStdString().c_str(), IMAGE_WIDTH, m_imageHeight, m_frameCountPerSec);
    m_miniVideoWriter->setup(m_miniVideoName.toStdString().c_str(), MINI_VIDEO_WIDTH, m_miniVideoHeight, m_frameCountPerSec);
}

VideoCreator::~VideoCreator()
{
    delete m_videoWriter;
    delete m_miniVideoWriter;
}
void VideoCreator::write()
{
    QDateTime now = QDateTime::currentDateTime();

    if(now > m_startTime.addMSecs(m_timeTillTick))
    {
        end();
    }
    else if(now >= m_startTime.addMSecs((m_frameNumber + 1)*15000/m_frameCountPerSec))
    {
        auto screens = QApplication::screens();

#ifdef __APPLE__
        auto geom = screens[m_screenId]->geometry();
        QPixmap screenShot = screens[m_screenId]->grabWindow(0, geom.x(), geom.y(), geom.width(), geom.height());
#else
        QPixmap screenShot = screens[m_screenId]->grabWindow(0);
#endif
        QImage  image = screenShot.toImage();

        QImage miniVideoImage = image.scaled(MINI_VIDEO_WIDTH, m_miniVideoHeight, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
        image = image.scaled(IMAGE_WIDTH, m_imageHeight, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);
        image = image.convertToFormat(QImage::Format_RGB888);
        miniVideoImage = miniVideoImage.convertToFormat(QImage::Format_RGB888);

        uchar* imageData = nullptr;
        uchar* miniVideoImageData = nullptr;
        if(m_videoWriter->m_picture_rgb24->linesize[0] != 3 * IMAGE_WIDTH){
            imageData = getAlignedImgForFFMPEG(image, m_videoWriter);
            image = QImage(imageData, IMAGE_WIDTH, m_imageHeight, m_videoWriter->m_picture_rgb24->linesize[0], QImage::Format_RGB888);
        }
        if(m_miniVideoWriter->m_picture_rgb24->linesize[0] != 3 * MINI_VIDEO_WIDTH){
            miniVideoImageData = getAlignedImgForFFMPEG(miniVideoImage, m_miniVideoWriter);
            miniVideoImage = QImage(miniVideoImageData, MINI_VIDEO_WIDTH, m_miniVideoHeight, m_miniVideoWriter->m_picture_rgb24->linesize[0], QImage::Format_RGB888);
        }
        m_videoWriter->addFrame(image.bits());
        m_miniVideoWriter->addFrame(miniVideoImage.bits());
        m_frameNumber++;
        if(imageData) delete[] imageData;
        if(miniVideoImageData) delete[] miniVideoImageData;
    }
}

void VideoCreator::end(bool isStopped)
{
    m_videoWriter->close();
    m_miniVideoWriter->close();
    VideoFilesInfo info;
    info.videoFileName = m_fileName;
    info.miniVideoFileName = m_miniVideoName;
    info.screenId = m_screenId;
    emit gifPrepared(isStopped, info);
}

uchar* VideoCreator::getAlignedImgForFFMPEG(QImage& img, FFMPEGVideoWriter* videoWriter)
{
    int c =0;
    uchar* imgForFFMPEG = new uchar[videoWriter->m_size];
    for ( int row = 0; row < img.height(); row++ ){
        for ( int col = 0; col < img.width(); col++ )
        {
            QColor clrCurrent( img.pixel( col, row ) );
            imgForFFMPEG[c] = clrCurrent.red();
            imgForFFMPEG[c+1] = clrCurrent.green();
            imgForFFMPEG[c + 2] = clrCurrent.blue();
            c+= 3;

        }
        c += videoWriter->m_picture_rgb24->linesize[0] - (3 * img.width());
    }
    return imgForFFMPEG;
}
