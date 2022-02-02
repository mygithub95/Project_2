#include "ControlTracker.h"
#include "ApiManager.h"
#include "TrackingManager.h"
#include "Logger.h"

#include <QMessageBox>

ControlTracker::ControlTracker(QObject *parent)
    : QObject(parent)
{
    createMembers();
    makeConnections();
}

ControlTracker::~ControlTracker()
{}

QString ControlTracker::getStorageBucketNames(const QString &credentials)
{
    return m_trackingManager->getStorageBucketNames(credentials);
}

void ControlTracker::createMembers()
{
    m_trackingManager = new TrackingManager(this);
}

void ControlTracker::makeConnections()
{
    connect(ApiManager::instance(), &ApiManager::settingInfoReady, this, &ControlTracker::onSettingsGetRequestFinished);
    connect(ApiManager::instance(), &ApiManager::badRequest, this, &ControlTracker::logErrorMessage);
    connect(ApiManager::instance(), &ApiManager::gettingSettingsInfoFailed, this, &ControlTracker::resetPage);
}

void ControlTracker::startOrStopTrackerSlot(const QString& message)
{
    if(message == "Stop tracking"){
        if(!m_trackingManager->isActive()) return;
        m_trackingManager->stop();
        Logger::instance()->log("Tracker is stopped");
    }
    else if(message == "Start tracking"){
        if(m_trackingManager->isActive()) return;
        ApiManager::instance()->requestToGetSettingStates();
        Logger::instance()->log("Tracker is started");
    }
}

void ControlTracker::onLogOutSlot()
{
    if(m_trackingManager->isActive())
        startOrStopTrackerSlot("Stop tracking");
    ApiManager::instance()->requestSignOut();
}

void ControlTracker::credentialsReceivedSlot(const QString &credentials)
{
    m_trackingManager->setCloudCredentials(credentials);
}

void ControlTracker::bucketNameReceivedSlot(const QString &bucketName)
{
    m_trackingManager->setCloudBucketName(bucketName);
}

void ControlTracker::onSettingsGetRequestFinished()
{
    auto settingsInfo = ApiManager::instance()->getSettingStatesInfo();
    m_trackingManager->initializeSettingParameters(settingsInfo);
    m_trackingManager->start();
}

void ControlTracker::logErrorMessage(const QString msg)
{
    Logger::instance()->log(msg);
}

void ControlTracker::showErrorMessage(const QString msg)
{
    QMessageBox box;
    box.setText(msg);
    box.exec();
}

void  ControlTracker::resetPage()
{
    logErrorMessage("Could not get setting parameters.");
    showErrorMessage("Can not start tracking.\n Please start again.");
}
