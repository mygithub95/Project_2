#ifndef TRACKINGMANAGER_H
#define TRACKINGMANAGER_H

#include <QObject>
#include "Info.h"

class AppInfoProvider;
class ScreenRecorderThread;
class CustomTimer;
class ActivityCalculator;
class QTimer;
class GoogleCloudManager;

class TrackingManager : public QObject
{
    Q_OBJECT
public:
    explicit TrackingManager(QObject *parent = nullptr);
    ~TrackingManager();

    void start();
    void stop();
    void checkToAppStartInfoProvider();
    bool isActive() const;
    void initializeSettingParameters(SettingStatesInfo info);
    void setCaptureScreen(bool b);
    void setTrackApps(bool b);
    void setTrackWebsites(bool b);
    void setCloudCredentials(const QString& credentials);
    void setCloudBucketName(const QString& bucketName);
    QString getStorageBucketNames(const QString& credentials);

private slots:
    void startScreenRecordSlot(const QString& startTime);
    void onScreenRecordFinishedSlot(VideoFilesInfo info);
    void clearDataAndStartRecordinSlot();

private:
    ActivityCalculator* m_activityCalculator;

    bool m_timeTrackerIsActive;
    bool m_captureScreen;
    bool m_trackApps;
    bool m_trackWebsites;

    QTimer* m_timerWorkingTime;
    AppInfoProvider* m_appInfoProvider;

    GoogleCloudManager* m_googleCloudManager;

    std::map<int, ScreenRecorderThread*> m_screenRecordersMap;
    std::vector<std::pair<VideoFilesInfo, std::vector<QByteArray>>> m_videoDatas;
    CustomTimer* m_captureScreenTimer;

    QString m_credentials;
    QString m_bucketName;
};

#endif // TRACKINGMANAGER_H
