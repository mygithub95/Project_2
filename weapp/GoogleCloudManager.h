#ifndef GOOGLECLOUDMANAGER_H
#define GOOGLECLOUDMANAGER_H

#include <QObject>

#include "Info.h"
#include "google/cloud/storage/client.h"

namespace gcs = ::google::cloud::storage;

class GoogleCloudManager : public QObject
{
    Q_OBJECT
public:
    GoogleCloudManager(QObject *parent = nullptr);

    void uploadFileToStorage(std::vector<std::pair<VideoFilesInfo, std::vector<QByteArray>>> videoDatas);
    void initializeCloudStorage(const QString& bucketName, const QString& jsonData);
    QString getBucketNames(const QString& jsonData);

private:
    QString m_bucketName;

    gcs::Client m_client;
};

#endif // GOOGLECLOUDMANAGER_H
