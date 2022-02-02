#include "MainWindow.h"
#include "DesktopWindow.h"
#include "ApiManager.h"
#include "ControlTracker.h"
#include "FrameWidget.h"
#include "Logger.h"
#include "AppUpdateController.h"
#include "WebViewInterface.h"
#include "LaunchAppController.h"

#ifdef __APPLE__
#include "MacDockHandler.h"
#else
#include "SystemTrayHandler.h"
#endif

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    resize(1200, 800);
    createMembers();
    makeConnections();
    setupLayout();
    installStylesheets();
    Logger::instance()->log("The APP version is " + QString::number(VERSION));
}

void MainWindow::createMembers()
{
    m_stackedWidget = new QStackedWidget(this);

    m_noInternetConnectionWidget = new QWidget(m_stackedWidget);
    m_desktopWindow = new DesktopWindow(m_stackedWidget);

    m_controlTracker = new ControlTracker(this);
    m_appUpdateController = new AppUpdateController(this);

    m_labelImage = new FrameWidget(m_noInternetConnectionWidget);
    m_labelImage->setImage(QPixmap(":/images/image.png"));
    m_labelText = new QLabel(m_noInternetConnectionWidget);
    m_labelText->setText("We couldn't find a secure connection to server!");
    m_labelText->setAlignment(Qt::AlignCenter);

    m_stackedWidget->addWidget(m_desktopWindow);
    m_stackedWidget->addWidget(m_noInternetConnectionWidget);

#ifdef __APPLE__
    m_appTrayIconHandler = new MacDockHandler(this);
#else
    m_appTrayIconHandler = new SystemTrayHandler(this);
#endif
}

void MainWindow::makeConnections()
{
    connect(m_appTrayIconHandler,   &AppTrayIconHandler::iconActivatedSignal, this, &MainWindow::iconActivatedSlot);
    connect(m_appTrayIconHandler,   &AppTrayIconHandler::actionTriggeredSignal, this, &MainWindow::onActionTriggeredSlot);
    connect(m_desktopWindow, &DesktopWindow::trackingTimeObtainedSignal, m_appTrayIconHandler, &AppTrayIconHandler::setIconTrackingTime);
    connect(m_desktopWindow, &DesktopWindow::tokenIsReady, this, &MainWindow::takeTokenAndSignInSlot);
    connect(m_desktopWindow, &DesktopWindow::internetConectionIsChecked, this, [this](bool result){
       if(result == m_stackedWidget->currentIndex())
           m_stackedWidget->setCurrentIndex(!result);
    });
    connect(m_desktopWindow, &DesktopWindow::startOrStopTrackerSignal, this, &MainWindow::startOrStopTrackerSlot);
    connect(m_desktopWindow, &DesktopWindow::updateApplicationSignal, m_appUpdateController, &AppUpdateController::updateApplicationSlot);
    connect(m_desktopWindow, &DesktopWindow::credentialsReceivedSignal, m_controlTracker, &ControlTracker::credentialsReceivedSlot);
    connect(m_desktopWindow, &DesktopWindow::bucketNameReceivedSignal, m_controlTracker, &ControlTracker::bucketNameReceivedSlot);
    connect(m_desktopWindow, &DesktopWindow::notificationReceivedSignal, m_appTrayIconHandler, &AppTrayIconHandler::showNotification);
    connect(m_desktopWindow, &DesktopWindow::credentialsUploadedSignal, this, &MainWindow::credentialsUploadedSlot);
    connect(m_desktopWindow, &DesktopWindow::onAutoStartSignal, this, &MainWindow::onAutoStartSlot);
    connect(m_appUpdateController, &AppUpdateController::updateIsAvailable, this, [this](QString version){
        if(!version.isEmpty()){
            Logger::instance()->log("Update is available. Version is == " + version);
            QStringList list = version.split('.');
            version = list[0] + "."+list[1] + list[2];
            QString jsForm = "setUpdateIsAvailable(\"" + version + "\")";
            m_desktopWindow->getWebViewInterface()->runJavaScript(jsForm);
        }
        else{
            Logger::instance()->log("Version is empty. No updates!");
            m_desktopWindow->getWebViewInterface()->runJavaScript("setUpdateIsAvailable(\"\")");
        }
    });
}

void MainWindow::setupLayout()
{
    QVBoxLayout *internetNoConnectionLayout = new QVBoxLayout;
    internetNoConnectionLayout->setSpacing(0);
    internetNoConnectionLayout->setMargin(0);
    internetNoConnectionLayout->addWidget(m_labelText, 1);
    internetNoConnectionLayout->addWidget(m_labelImage, 4);
    m_noInternetConnectionWidget->setLayout(internetNoConnectionLayout);

    QHBoxLayout* stackLayout = new QHBoxLayout;
    stackLayout->setMargin(0);
    stackLayout->setSpacing(0);
    stackLayout->addWidget(m_stackedWidget);
    this->setLayout(stackLayout);
}

void MainWindow::installStylesheets()
{
    m_labelText->setStyleSheet("font-size: 17px;");
}

void MainWindow::takeTokenAndSignInSlot(const QString& token)
{

    if(token != ""){
        Logger::instance()->log("Token is recived: size = " + QString::number(token.size()));
        QString privateToken = ApiManager::instance()->getTokenValue();
        if(token == privateToken ){
            Logger::instance()->log("The token is same.");
            QTimer::singleShot(1000, this, &MainWindow::runJsToGetTimeForIcon);
            return;
        }
        if(!privateToken.isEmpty()){
            Logger::instance()->log("Another token is received. LogOut would be called.");
            m_controlTracker->onLogOutSlot();
            m_appTrayIconHandler->enableActionsAndIconTime(false);
        }
        ApiManager::instance()->setTokenValue(token);
        ApiManager::instance()->signIn();
        m_appUpdateController->checkForUpdates();
        m_appTrayIconHandler->enableActionsAndIconTime(true);
        QTimer::singleShot(1000, this, &MainWindow::runJsToGetTimeForIcon);
        Logger::instance()->log("Token received. Signed in");
    }else{
        Logger::instance()->log("Token is empty. LogOut would be called.");
        m_controlTracker->onLogOutSlot();
        m_appTrayIconHandler->enableActionsAndIconTime(false);
    }
}

void MainWindow::credentialsUploadedSlot(const QString &credentials)
{
    Logger::instance()->log("MainWindow::credentialsUploadedSlot: Credentials uploaded size = " + QString::number(credentials.size()));
    const QString& bucketNames =  m_controlTracker->getStorageBucketNames(credentials);
    m_controlTracker->bucketNameReceivedSlot("");
    m_desktopWindow->getWebViewInterface()->runJavaScript("getListOfUserBuckets(\"" + bucketNames +"\")");
    Logger::instance()->log("MainWindow::credentialsUploadedSlot: Send bucket names to server. Bucket names = " + bucketNames);
}

void MainWindow::iconActivatedSlot(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::ActivationReason::Trigger)
    {
        m_desktopWindow->getWebViewInterface()->loadPage();
        showNormal();
    }
}

void MainWindow::startOrStopTrackerSlot(const QString& message)
{
    if(message == "Start tracking"){
        m_appTrayIconHandler->setWorkingActionText("Stop Working");
        m_appTrayIconHandler->startIconTimer(true);
    }
    else{
        m_appTrayIconHandler->setWorkingActionText("Start Working");
        m_appTrayIconHandler->startIconTimer(false);
        QTimer::singleShot(1000, this, &MainWindow::runJsToGetTimeForIcon);
    }
    m_controlTracker->startOrStopTrackerSlot(message);
}

void MainWindow::onStartStopActionSlot(const QString& actionText)
{
    if(actionText == "Start Working")
    {
        Logger::instance()->log("MainWindow::onStartStopActionSlot(): Start Working.");
        m_appTrayIconHandler->setWorkingActionText("Stop Working");
        m_controlTracker->startOrStopTrackerSlot("Start tracking");
        m_desktopWindow->getWebViewInterface()->runJavaScript("changeRunningAppStatus()");
        m_appTrayIconHandler->startIconTimer(true);
    }
    else
    {
        Logger::instance()->log("MainWindow::onStartStopActionSlot(): Stop Working.");
        m_appTrayIconHandler->setWorkingActionText("Start Working");
        m_controlTracker->startOrStopTrackerSlot("Stop tracking");
        m_desktopWindow->getWebViewInterface()->runJavaScript("changeRunningAppStatus()");
        m_appTrayIconHandler->startIconTimer(false);
        QTimer::singleShot(1000, this, &MainWindow::runJsToGetTimeForIcon);
    }
}

void MainWindow::onActionTriggeredSlot(const QString& actionText)
{
    if(actionText == "Start Working" || actionText == "Stop Working")
    {
        onStartStopActionSlot(actionText);
    }
    else if(actionText == "Open the dashboard")
    {
        Logger::instance()->log("MainWindow::onActionTriggeredSlot(): Open the dashboard.");
        m_desktopWindow->getWebViewInterface()->loadPage("https://internal.weapp.io/src/modules/dashboard?");
        showNormal();
    }
    else if(actionText == "Help Center")
    {
        Logger::instance()->log("MainWindow::onActionTriggeredSlot(): Help Center.");
        QDesktopServices::openUrl(QUrl("http://help.weapp.io/"));
    }
    else if(actionText == "About WEAPP")
    {
        Logger::instance()->log("MainWindow::onActionTriggeredSlot(): About WEAPP.");
        QDesktopServices::openUrl(QUrl("https://weapp.io/about_us"));
    }
    else if(actionText == "Preferences")
    {
        Logger::instance()->log("MainWindow::onActionTriggeredSlot(): Preferences.");
        m_desktopWindow->getWebViewInterface()->loadPage();
        m_desktopWindow->getWebViewInterface()->runJavaScript("showUserPreferences()");
        showNormal();
    }
}

AppTrayIconHandler* MainWindow::getAppTrayIconHandler()
{
    return m_appTrayIconHandler;
}

void MainWindow::runJsToGetTimeForIcon()
{
#ifdef __APPLE__
    Logger::instance()->log("MainWindow::runJsToGetTimeForIcon(): call getTrackingTime()");
    m_desktopWindow->getWebViewInterface()->runJavaScript("getTrackingTime()", JsRunType::GETTIME);
#endif
}

void MainWindow::onAutoStartSlot(bool on)
{
    Logger::instance()->log("MainWindow::onAutoStartSlot: autostart is " + QString::number(on));
    LaunchAppController launch;
    on ? launch.addAppOnStartup() : launch.removeAppFromStartup();
}
