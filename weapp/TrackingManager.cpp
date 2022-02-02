#include "TrackingManager.h"
#include "CustomTimer.h"
#include "ApiManager.h"
#include "ScreenRecorderThread.h"
#include "ActivityCalculator.h"
#include "Info.h"
#include "GoogleCloudManager.h"
#include "Logger.h"

#ifdef _WIN32
#include "WindowsAppInfoProvider.h"
#elif __linux__
#include "LinuxAppInfoProvider.h"
#elif __APPLE__
#include "MacAppInfoProvider.h"
#endif

#include <QGuiApplication>
#include <QTimer>
#include <QDir>
#include <QDateTime>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

TrackingManager::TrackingManager(QObject *parent)
    : QObject(parent)
    , m_timeTrackerIsActive(false)
    , m_captureScreen(true)
    , m_trackApps(true)
    , m_trackWebsites(true)
{
    m_captureScreenTimer = new CustomTimer(this);
    m_activityCalculator = new ActivityCalculator(this);
    m_googleCloudManager = new GoogleCloudManager(this);
#ifdef _WIN32
    m_appInfoProvider = new WindowsAppInfoProvider;
#elif __linux__
    m_appInfoProvider = new LinuxAppInfoProvider;
#elif __APPLE__
    m_appInfoProvider = new MacAppInfoProvider;
#endif

    m_timerWorkingTime = new QTimer(this);
    m_timerWorkingTime->setInterval(20000);
    connect(m_timerWorkingTime, &QTimer::timeout, ApiManager::instance(), &ApiManager::updateWorkingTime);

    connect(m_captureScreenTimer, &CustomTimer::startScreenRecord, this, &TrackingManager::startScreenRecordSlot);
    connect(m_appInfoProvider, &AppInfoProvider::sendInfo, ApiManager::instance(), &ApiManager::updatesScreenInfo);
    connect(m_activityCalculator, &ActivityCalculator::activityIsReady, ApiManager::instance(), &ApiManager::sendActivityPercentage);
}

TrackingManager::~TrackingManager()
{
    if(m_appInfoProvider)
        delete m_appInfoProvider;
}

void TrackingManager::setCaptureScreen(bool b)
{
    if(m_captureScreen == b) return;
    m_captureScreen = b;
    if (m_timeTrackerIsActive) {
        m_captureScreenTimer->stop();
        for(auto timerForScreenRecord : m_screenRecordersMap)
            timerForScreenRecord.second->stop();
        m_screenRecordersMap.clear();
        if(m_captureScreen){
            m_captureScreenTimer->start();
        }
    }
}

void TrackingManager::setTrackApps(bool b)
{
    if(m_trackApps == b) return;
    m_trackApps = b;
    if (m_timeTrackerIsActive) {
        m_appInfoProvider->stop();
        checkToAppStartInfoProvider();
    }
}

void TrackingManager::setTrackWebsites(bool b)
{
    if(m_trackWebsites == b) return;
    m_trackWebsites = b;
    if (m_timeTrackerIsActive) {
        m_appInfoProvider->stop();
        checkToAppStartInfoProvider();
    }
}

void TrackingManager::setCloudCredentials(const QString &credentials)
{
    m_credentials = credentials;
    Logger::instance()->log("TrackingManager::setCloudCredentials: m_credentials received, size == " + QString::number(m_credentials.size()));
    if(!m_credentials.isEmpty() && !m_bucketName.isEmpty()){
        Logger::instance()->log("TrackingManager::setCloudCredentials: start cloud initialization");
        m_googleCloudManager->initializeCloudStorage(m_bucketName, m_credentials);
    }
}

void TrackingManager::setCloudBucketName(const QString &bucketName)
{
    m_bucketName = bucketName;
    Logger::instance()->log("TrackingManager::setCloudBucketName: m_bucketName received = " + m_bucketName);
    if(!m_credentials.isEmpty() && !m_bucketName.isEmpty()){
        Logger::instance()->log("TrackingManager::setCloudBucketName: start cloud initialization");
        m_googleCloudManager->initializeCloudStorage(m_bucketName, m_credentials);
    }
}

QString TrackingManager::getStorageBucketNames(const QString &credentials)
{
    return m_googleCloudManager->getBucketNames(credentials);
}

void TrackingManager::start()
{
    checkToAppStartInfoProvider();

    if (m_captureScreen) {
        m_videoDatas.clear();
        m_captureScreenTimer->start();
    }
    m_activityCalculator->startActivityCalculation();
    m_timerWorkingTime->start();
    m_timeTrackerIsActive = true;
}

void TrackingManager::stop()
{
    for(auto timerForScreenRecord : m_screenRecordersMap){
        timerForScreenRecord.second->stop();
    }
    m_screenRecordersMap.clear();
    m_activityCalculator->stopActivityCalculation();
    m_appInfoProvider->stop();
    m_captureScreenTimer->stop();
    m_timerWorkingTime->stop();
    m_timeTrackerIsActive = false;
}

bool TrackingManager::isActive() const
{
    return m_timeTrackerIsActive;
}

void TrackingManager::startScreenRecordSlot(const QString& startTime)
{
    auto screens = QGuiApplication::screens();
    QDir dir(QDir::tempPath() + "/TimeTracker");
    if(!dir.exists()){
        dir.mkpath(".");
    }
    for(int i = 0; i < screens.size(); i++){
        ScreenRecorderThread* timerForScreenRecord = new ScreenRecorderThread(i, 60, 5);
        m_screenRecordersMap[i] = timerForScreenRecord;
        timerForScreenRecord->start();
        connect(timerForScreenRecord, &ScreenRecorderThread::screenRecordReady, this, [this, startTime](VideoFilesInfo info){
            info.startTime = startTime;
            onScreenRecordFinishedSlot(info);
        });
    }
}

void TrackingManager::onScreenRecordFinishedSlot(VideoFilesInfo info)
{
    m_screenRecordersMap.erase(info.screenId);
    std::pair<VideoFilesInfo, std::vector<QByteArray>> pair;
    pair.first = info;
    if(m_credentials.isEmpty() || m_bucketName.isEmpty()){
        Logger::instance()->log("Read video files from path ------- > " + info.videoFileName);
        Logger::instance()->log("Read video files from path ------- > " + info.miniVideoFileName);
        pair.second.push_back(ApiManager::instance()->readFile(info.videoFileName));
        pair.second.push_back(ApiManager::instance()->readFile(info.miniVideoFileName));
    }
    m_videoDatas.push_back(pair);
    if(m_screenRecordersMap.empty() && m_timeTrackerIsActive){
        if(m_credentials.isEmpty() || m_bucketName.isEmpty()){
            ApiManager::instance()->sendScreenRecordToServer(m_videoDatas);
            clearDataAndStartRecordinSlot();
        }
        else{
            QFutureWatcher<void>* futureWatcher = new QFutureWatcher<void>;
            futureWatcher->setFuture(QtConcurrent::run(m_googleCloudManager, &GoogleCloudManager::uploadFileToStorage, m_videoDatas));
            connect(futureWatcher, &QFutureWatcher<void>::finished, this, &TrackingManager::clearDataAndStartRecordinSlot);
        }
    }
}

void TrackingManager::clearDataAndStartRecordinSlot()
{
    QFutureWatcher<void>* futureWatcher = dynamic_cast<QFutureWatcher<void>*>(sender());
    if (futureWatcher) {
        futureWatcher->deleteLater();
    }
    m_videoDatas.clear();
    startScreenRecordSlot(QDateTime::currentDateTime().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz"));
}

void TrackingManager::checkToAppStartInfoProvider()
{
    if (m_trackApps || m_trackWebsites) {
        m_appInfoProvider->setAppInfoEnabled(m_trackApps);
        m_appInfoProvider->setBrowserInfoEnabled(m_trackWebsites);
        m_appInfoProvider->start();
    }
}

void TrackingManager::initializeSettingParameters(SettingStatesInfo info)
{
    m_captureScreen = info.screenRecordEnabled;
    m_trackApps = info.appsEnabled;
    m_trackWebsites = info .websitesEnabled;
}
