#include "ApiManager.h"
#include "ImageDownloader.h"
#include "Logger.h"

#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QSettings>
#include <QBuffer>
#include <QImageWriter>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QApplication>
#include <QDir>
#include <QUrlQuery>

namespace{
QByteArray  readImageFromQPixmap(const QPixmap& pixmap){

    QImage image = pixmap.toImage();
    QByteArray imgData;
    QBuffer buffer(&imgData);

    // write image to memory buffer
    QImageWriter writer;
    writer.setDevice(&buffer);
    writer.setFormat("JPEG");
    writer.setCompression(9);
    writer.write(image);

    return imgData;
}

}

ApiManager::ApiManager()
    : m_token("")
    , m_userId(-1)
{
    //-------------for test-------------
    //QDir dir(QApplication::applicationDirPath() + "/Image");
    //if(!dir.exists()){
    //    dir.mkpath(".");
    //}
    //----------------------------------
}

ApiManager *ApiManager::instance()
{
    static ApiManager* instance = nullptr;
    if(!instance){
        instance = new ApiManager;
    }
    return instance;
}

SettingStatesInfo ApiManager::getSettingStatesInfo()
{
    return m_settingStatesInfo;
}

void ApiManager::requestSignOut()
{
    m_token = "";
    m_userId = -1;
}

void ApiManager::sendActivityPercentage(const QString& percentageOfActivity)
 {
    QUrl url("https://api.weapp.io/api/time_tracker/activities");
    Logger::instance()->log("percentageOfActivity ======== " + percentageOfActivity);
    QByteArray percentage(percentageOfActivity.toUtf8());
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart activityPart;
    activityPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"percentage\";")));
    activityPart.setBody(percentage);
    multiPart->append(activityPart);

    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("authorization"), m_token.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + multiPart->boundary());
    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(this);
    multiPart->setParent(networkAccessManager->post(request, multiPart));
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &ApiManager::activityPercentageSendingFinished);
}

void ApiManager::signIn()
{
    QUrl url("https://api.weapp.io/api/init/sessions/xx");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArray("authorization"), m_token.toUtf8());

    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(this);
    networkAccessManager->get(request);
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &ApiManager::onRequestSignInFinishedSlot);
}

QByteArray ApiManager::readFile(const QString &fileName)
{
    QByteArray fileData;
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        return QByteArray();
    }
    fileData = file.readAll();
    file.close();
    file.remove();
    return fileData;
}

void ApiManager::updatesScreenInfo(std::vector<AppAndBrowserInfo> info)
{
    if(info.empty()){
        return;
    }
    QUrl url("https://api.weapp.io/api/time_tracker/app_times/xx");
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    for(size_t i = 0; i < info.size(); ++i){
        Logger::instance()->log("AppAndBrowserInfo name --> " + info.at(i).name);
        Logger::instance()->log("AppAndBrowserInfo type --> " + info.at(i).type);
        Logger::instance()->log("AppAndBrowserInfo icon --> " + QString::number(!info.at(i).icon.isNull()));
        Logger::instance()->log("AppAndBrowserInfo time --> " + QString::number(info.at(i).runDuration));
        if(info.at(i).name.isEmpty() || info.at(i).type.isEmpty())
            continue;

        QByteArray iconData = readImageFromQPixmap(info.at(i).icon);
        //-------------for test-------------
        //QString iconName = info.at(i).name.length() > 30 ? "browser" + QString::number(i) : info.at(i).name;
        //info.at(i).icon.save(QApplication::applicationDirPath() + "/Image/" + iconName + ".png");
        //----------------------------------
        QByteArray type(info.at(i).type.toUtf8());
        QHttpPart typePart;
        typePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"apps[][:type]\";")));
        typePart.setBody(type);
        multiPart->append(typePart);

        QByteArray name(info.at(i).name.toUtf8());
        QHttpPart namePart;
        namePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"apps[][:name]\";")));
        namePart.setBody(name);
        multiPart->append(namePart);

        QHttpPart iconPart;
        iconPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
        iconPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"apps[][:ico]\"; filename=\"appIcon.jpg\"")));
        iconPart.setBody(iconData);
        multiPart->append(iconPart);

        QByteArray duration(QString::number(info.at(i).runDuration).toUtf8());
        QHttpPart durationPart;
        durationPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"apps[][:duration]\";")));
        durationPart.setBody(duration);
        multiPart->append(durationPart);
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArray("authorization"), m_token.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + multiPart->boundary());

    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(this);
    multiPart->setParent(networkAccessManager->put(request, multiPart));
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &ApiManager::onUpdateSeasonBeatFinished);
}

void ApiManager::updateWorkingTime()
{
    QUrl url("https://api.weapp.io/api/time_tracker/work_times/xx");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArray("authorization"), m_token.toUtf8());

    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(this);
    networkAccessManager->put(request, QByteArray());
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &ApiManager::onRequestupdateWorkingTimeSlot);
}

void ApiManager::sendScreenRecordToServer(const std::vector<std::pair<VideoFilesInfo, std::vector<QByteArray> > > &videoDatas)
{
    QUrl url("https://api.weapp.io/api/time_tracker/screenshots/xx");

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    for(auto videoData : videoDatas){

        QHttpPart videoPart;
        videoPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("video/mp4"));
        videoPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"screenshots[][:file]\"; filename=\"screenRecord.mp4\"")));
        videoPart.setBody(videoData.second[0]);
        multiPart->append(videoPart);
        Logger::instance()->log("Added in multipart screenRecord with size  == " + QString::number(videoData.second[0].size()));

        QHttpPart miniVideoPart;
        miniVideoPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("video/mp4"));
        miniVideoPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"screenshots[][:file_s]\"; filename=\"screenRecord_s.mp4\"")));
        miniVideoPart.setBody(videoData.second[1]);
        multiPart->append(miniVideoPart);
        Logger::instance()->log("Added in multipart miniScreenRecord with size  == " + QString::number(videoData.second[1].size()));

        QByteArray screen_id(QString::number(videoData.first.screenId).toUtf8());
        QHttpPart screenIdPart;
        screenIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"screenshots[][:monitor]\";")));
        screenIdPart.setBody(screen_id);
        multiPart->append(screenIdPart);
    }
    QByteArray created_at(videoDatas[0].first.startTime.toUtf8());
    QHttpPart createdAtPart;
    createdAtPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"created_at\";")));
    createdAtPart.setBody(created_at);
    multiPart->append(createdAtPart);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArray("authorization"), m_token.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + multiPart->boundary());

    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(this);
    multiPart->setParent(networkAccessManager->put(request, multiPart));
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &ApiManager::onUploadedScreenShotFinished);
}

void ApiManager::onRequestSignInFinishedSlot(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError){
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (document.isObject()) {
            QJsonObject object = document.object();
            QJsonObject userObject;
            QJsonValue  objVal;

            objVal = object.value("user");
            userObject = objVal.toObject();
            m_userId = userObject.value("id").toInt();
            Logger::instance()->log("User Id get successfully.");
        }
    }
    else{
         emit badRequest("UserId getting error. \n" + reply->errorString());
    }
    dynamic_cast<QNetworkAccessManager *>(QObject::sender())->deleteLater();
    reply->deleteLater();
}

void ApiManager::onUpdateSeasonBeatFinished(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError){
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (document.isObject()) {
            QJsonObject obj = document.object();
            qDebug() << "App Info upload status : " << obj.value("screenshot").toString();
            Logger::instance()->log("App Info upload status : " + obj.value("screenshot").toString());
        }
    }
    else{
        emit badRequest("App Info upload error.\n" + reply->errorString());
    }
    dynamic_cast<QNetworkAccessManager *>(QObject::sender())->deleteLater();
    reply->deleteLater();
}

void ApiManager::onUploadedScreenShotFinished(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError){
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (document.isObject()) {
            QJsonObject obj = document.object();
            qDebug() << "Image upload status: " << obj.value("screenshot").toString();
            Logger::instance()->log("Image upload status: " + obj.value("screenshot").toString());
        }
    }
    else{
        emit badRequest("Image uplaod error.\n" + reply->errorString());
    }
    dynamic_cast<QNetworkAccessManager *>(QObject::sender())->deleteLater();
    reply->deleteLater();
}

void ApiManager::onRequestupdateWorkingTimeSlot(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError){
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (document.isObject()) {
            QJsonObject obj = document.object();
            qDebug() << "Working time status: " << obj.value("screenshot").toString();
            Logger::instance()->log("Working time status: " + obj.value("screenshot").toString());
        }
    }
    else{
        emit badRequest("Working time error.\n" + reply->errorString());
    }
    dynamic_cast<QNetworkAccessManager *>(QObject::sender())->deleteLater();
    reply->deleteLater();
}

void ApiManager::updateSettingsFromJsonDocument(const QJsonDocument &doc)
{
    //document should be valid!
    QJsonObject object = doc.object();
    QJsonObject settingsObject;
    settingsObject = object.value("settings").toObject();
    m_settingStatesInfo.permissionEnabled = settingsObject.value("editing").toBool();
    m_settingStatesInfo.screenRecordEnabled = settingsObject.value("screen").toBool();
    m_settingStatesInfo.appsEnabled= settingsObject.value("apps").toBool();
    m_settingStatesInfo.websitesEnabled = settingsObject.value("website").toBool();
}

void ApiManager::activityPercentageSendingFinished(QNetworkReply *reply){
    if(reply->error() == QNetworkReply::NoError){
        qDebug()<<"Activity percentage was succesfully sended.";
        Logger::instance()->log("Activity percentage was succesfully sended.");
    }else{
        emit badRequest("Sending activity percentage error.\n" + reply->errorString());
    }
    dynamic_cast<QNetworkAccessManager *>(QObject::sender())->deleteLater();
    reply->deleteLater();
}

void ApiManager::setTokenValue(const QString& token)
{
    m_token = token;
}

QString ApiManager::getTokenValue()
{
    return m_token;
}

void ApiManager::requestToGetSettingStates()
{
    QUrl url("https://api.weapp.io/api/time_tracker/settings");
    QUrlQuery query;
    query.addQueryItem("user_id" , QString::number(m_userId));
    url.setQuery(query.query());
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArray("authorization"), m_token.toUtf8());

    QNetworkAccessManager *networkAccessManager = new QNetworkAccessManager(this);
    networkAccessManager->get(request);
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &ApiManager::onRequestGetSettingFinishedSlot);
}

void ApiManager::onRequestGetSettingFinishedSlot(QNetworkReply* reply)
{
    if(reply->error() == QNetworkReply::NoError){
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (document.isObject()) {
           updateSettingsFromJsonDocument(document);
            emit settingInfoReady();
        }
    }
    else{
        emit gettingSettingsInfoFailed();
        emit badRequest("Setting parameters getting error.\n" + reply->errorString());
    }
    dynamic_cast<QNetworkAccessManager *>(QObject::sender())->deleteLater();
    reply->deleteLater();
}
