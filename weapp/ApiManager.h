#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QObject>
#include <QPixmap>
#include "Info.h"

class QNetworkReply;

class ApiManager : public QObject
{
    Q_OBJECT
public:
    static ApiManager* instance();

    //API calls
    void signIn();
    void requestSignOut();
    void sendScreenRecordToServer(const std::vector<std::pair<VideoFilesInfo, std::vector<QByteArray>>>& videoDatas);
    void setTokenValue(const QString& token);
    QString getTokenValue();
    QByteArray readFile(const QString& fileName);
    void requestToGetSettingStates();

    SettingStatesInfo getSettingStatesInfo();

private:
    explicit ApiManager();
    //remove copy constructor and assignment operator for Singleton
    ApiManager(const ApiManager&) = delete;
    ApiManager& operator=(const ApiManager&) = delete;

    void updateSettingsFromJsonDocument(const QJsonDocument& doc);

    QString m_token;
    int m_userId;
    SettingStatesInfo m_settingStatesInfo;

signals:
    void settingInfoReady();
    void badRequest(const QString msg);
    void gettingSettingsInfoFailed();

private slots:
    void onRequestSignInFinishedSlot(QNetworkReply* reply);
    void activityPercentageSendingFinished(QNetworkReply* reply);
    void onRequestGetSettingFinishedSlot(QNetworkReply* reply);

    void onUpdateSeasonBeatFinished(QNetworkReply* reply);
    void onUploadedScreenShotFinished(QNetworkReply* reply);
    void onRequestupdateWorkingTimeSlot(QNetworkReply* reply);

public slots:
    void updatesScreenInfo(std::vector<AppAndBrowserInfo> info);
    void updateWorkingTime();
    void sendActivityPercentage(const QString& percentageOfActivity);
};

#endif // APIMANAGER_H
