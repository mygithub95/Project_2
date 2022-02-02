#include "GoogleCloudManager.h"
#include "Logger.h"

#include <fstream>
#include <QStringList>

namespace {
void uploadFile(google::cloud::storage::Client client,
                std::vector<std::string> const& argv) {
    //! [upload file] [START storage_upload_file]q
    try {
        using ::google::cloud::StatusOr;
        [](gcs::Client client, std::string const& file_name,
                std::string const& bucket_name, std::string const& object_name) {
            // Note that the client library automatically computes a hash on the
            // client-side to verify data integrity during transmission.
            StatusOr<gcs::ObjectMetadata> metadata = client.UploadFile(
                        file_name, bucket_name, object_name, gcs::IfGenerationMatch(0), gcs::ContentType("video/mp4"));

            if (!metadata) throw std::runtime_error(metadata.status().message());
            Logger::instance()->log("Video upload status: " + QString::fromStdString(object_name) + " uploaded to cloud");
        }
        //! [upload file] [END storage_upload_file]
        (std::move(client), argv.at(0), argv.at(1), argv.at(2));
    }
    catch (std::exception e){
        Logger::instance()->log("Video upload status: exception " + QString(e.what()));
    }
}

void listBucketsForProject(google::cloud::storage::Client client,
                           std::vector<std::string> const& argv, std::string& bucketNames) {
    //! [list buckets for project]
    try{
        namespace gcs = ::google::cloud::storage;
        [](gcs::Client client, std::string const& project_id, std::string& bucketNames) {
            int count = 0;
        	for (auto&& bucket_metadata : client.ListBucketsForProject(project_id)) {
                if (!bucket_metadata) {
                    Logger::instance()->log("listBucketsForProject throws exception: " + QString::fromStdString(bucket_metadata.status().message()));
                    throw std::runtime_error(bucket_metadata.status().message());
                }
                bucketNames += bucket_metadata->name() + "#";
                ++count;
            }
            if (count == 0) {
                Logger::instance()->log("No buckets in project " + QString::fromStdString(project_id));
            }
        }
        //! [list buckets for project]
        (std::move(client), argv.at(0), bucketNames);
    }
    catch (std::exception e){
        Logger::instance()->log("listBucketsForProject: exception is " + QString(e.what()));
    }
}
}

GoogleCloudManager::GoogleCloudManager(QObject *parent)
    : QObject(parent)
{}

void GoogleCloudManager::uploadFileToStorage(std::vector<std::pair<VideoFilesInfo, std::vector<QByteArray>>> videoDatas)
{
    for(auto videoData : videoDatas){
        QString videoFilePath = videoData.first.videoFileName;
        QString miniVideoFilePath = videoData.first.miniVideoFileName;

        QString videoFileName = videoFilePath.mid(videoFilePath.lastIndexOf("/") + 1);
        QString miniVideoFileName = miniVideoFilePath.mid(miniVideoFilePath.lastIndexOf("/") + 1);

        QString objectName1 = videoFileName.left(videoFileName.length() - 4) + "_" + videoData.first.startTime + ".mp4";
        QString objectName2 = miniVideoFileName.left(miniVideoFileName.length() - 4) + "_" + videoData.first.startTime + ".mp4";

        uploadFile(m_client, {videoFilePath.toStdString(), m_bucketName.toStdString(), objectName1.toStdString()});
        uploadFile(m_client, {miniVideoFilePath.toStdString(), m_bucketName.toStdString(), objectName2.toStdString()});
    }
}

void GoogleCloudManager::initializeCloudStorage(const QString& bucketName, const QString& jsonData)
{
    m_bucketName = bucketName;
    auto credentials = google::cloud::MakeServiceAccountCredentials(jsonData.toStdString());
    m_client = gcs::Client(google::cloud::Options{}.set<google::cloud::UnifiedCredentialsOption>(credentials));
    Logger::instance()->log("GoogleCloudManager::initializeCloudStorage: Bucket name is " + m_bucketName);
}

QString GoogleCloudManager::getBucketNames(const QString& jsonData)
{
    Logger::instance()->log("GoogleCloudManager::getBucketNames: jsonData size = " + QString::number(jsonData.size()));
    initializeCloudStorage("", jsonData);
    auto jsonList = jsonData.split(',');
    std::string bucketNames;
    for(auto& data : jsonList)
    {
        if(data.contains("\"project_id\"")){
            QStringList list = data.split(":");
            if(list.size() < 2) {
                Logger::instance()->log("GoogleCloudManager::getBucketNames: list size is small. data == " + data);
                break;
            }
            QString projectId  = list[1].remove('"');
            projectId.remove(' ');
            Logger::instance()->log("GoogleCloudManager::getBucketNames: projectId = " + projectId);
            listBucketsForProject(m_client, {projectId.toStdString()}, bucketNames);
            break;
        }
    }
    QString names = QString::fromStdString(bucketNames);
    Logger::instance()->log("GoogleCloudManager::getBucketNames: bucketNames = " + names);
    return names;
}
