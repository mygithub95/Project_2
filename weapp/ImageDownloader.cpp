#include "ImageDownloader.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

ImageDownloader::ImageDownloader(int timeForExit)
    : QObject(), m_timeForExit(timeForExit)
{
    m_imageDownloader = new QNetworkAccessManager(this);
    connect(m_imageDownloader, &QNetworkAccessManager::finished, this, &ImageDownloader::onImageDownloaded);
}

void ImageDownloader::setUrl(const QString &url, ImageType type)
{
    if(url.isEmpty()){
        emit done(QPixmap());
        return ;
    }
    m_url = url;
    QUrl imageUrl;
    if(type == ImageType::eFavIcon){
        imageUrl = QUrl("https://f1.allesedv.com/64/" + url);
    }else{
        imageUrl = QUrl(url);
    }
    QNetworkRequest request(imageUrl);
    m_imageDownloader->get(request);
    QTimer::singleShot(m_timeForExit, this, SLOT(timeout()));
}

QString ImageDownloader::getUrl() const
{
    return m_url;
}

void ImageDownloader::onImageDownloaded(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError){
        QPixmap downloadedImage;
        downloadedImage.loadFromData(reply->readAll());
        if(!downloadedImage.isNull() && downloadedImage.size() != QSize(1, 1))
            emit done(downloadedImage);
        else
            emit done(QPixmap());
    }
    else
        emit done(QPixmap());
    m_imageDownloader->deleteLater();
    reply->deleteLater();
}

void ImageDownloader::timeout()
{
    emit done(QPixmap());
    this->deleteLater();
}

