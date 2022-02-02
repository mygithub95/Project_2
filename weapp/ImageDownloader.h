#ifndef IMAGEDOWNLOADER_H
#define IMAGEDOWNLOADER_H

#include <QObject>
#include <QPixmap>

class QNetworkAccessManager;
class QNetworkReply;

enum ImageType
{
    eFavIcon, eNormal
};

class ImageDownloader : public QObject
{
    Q_OBJECT
public:
    explicit ImageDownloader(int timeForExit);
    void setUrl(const QString& url, ImageType type = ImageType::eNormal);
    QString getUrl() const;
private:
    QNetworkAccessManager* m_imageDownloader;
    int m_timeForExit;
    QString m_url;
signals:
    void done(const QPixmap& image);

public slots:
    void onImageDownloaded(QNetworkReply *reply);
    void timeout();

};

#endif // IMAGEDOWNLOADER_H
