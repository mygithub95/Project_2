 #include "DesktopWindow.h"
#ifndef __APPLE__
#include "WebEngineView.h"
#else
#include "MacWebView.h"
#endif

#include "FrameWidget.h"
#include "ApiManager.h"

#include <QPushButton>
#include <QApplication>
#include <QScreen>
#include <QLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QTimer>
#include <QSettings>

DesktopWindow::DesktopWindow(QWidget *parent)
    : QWidget(parent)
{
    createMembers();
    makeConnections();
    setupLayout();
}

DesktopWindow::~DesktopWindow()
{
}

void DesktopWindow::createMembers()
{
    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this , &DesktopWindow::onTimerTimoutSlot);
    timer->start();

    m_widgetInternetConnection = new QWidget(this);

#ifndef __APPLE__
    m_webViewInterface = new WebEngineView(m_widgetInternetConnection);
#else
    m_webViewInterface = new MacWebView(m_widgetInternetConnection);
#endif
}

void DesktopWindow::makeConnections()
{
    connect(m_webViewInterface, &WebViewInterface::tokenReceived, this, &DesktopWindow::tokenIsReady);
    connect(m_webViewInterface, &WebViewInterface::onButtonStartOrStopPressed, this, &DesktopWindow::startOrStopTrackerSignal);
    connect(m_webViewInterface, &WebViewInterface::updateButtonPressed, this, &DesktopWindow::updateApplicationSignal);
    connect(m_webViewInterface, &WebViewInterface::connectionChecked, this, &DesktopWindow::internetConectionIsChecked);
    connect(m_webViewInterface, &WebViewInterface::credentialsUploaded, this, &DesktopWindow::credentialsUploadedSignal);
    connect(m_webViewInterface, &WebViewInterface::credentialsTriggered, this, &DesktopWindow::credentialsReceivedSignal);
    connect(m_webViewInterface, &WebViewInterface::bucketNameTriggered, this, &DesktopWindow::bucketNameReceivedSignal);
    connect(m_webViewInterface, &WebViewInterface::notificationTriggered, this, &DesktopWindow::notificationReceivedSignal);
    connect(m_webViewInterface, &WebViewInterface::trackingTimeObtained, this, &DesktopWindow::trackingTimeObtainedSignal);
    connect(m_webViewInterface, &WebViewInterface::onAutoStartSignal, this, &DesktopWindow::onAutoStartSignal);
}

void DesktopWindow::setupLayout()
{
    QVBoxLayout *internetConnectionLayout = new QVBoxLayout;
    internetConnectionLayout->setSpacing(0);
    internetConnectionLayout->setMargin(0);
    internetConnectionLayout->addWidget(m_webViewInterface);
    m_widgetInternetConnection->setLayout(internetConnectionLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_widgetInternetConnection);

    this->setLayout(mainLayout);
}

void DesktopWindow::onTimerTimoutSlot()
{
    QString jsform = "function isOnLine() {"
                             "return navigator.onLine;;"
                             "}"
                             "isOnLine()";

    m_webViewInterface->runJavaScript(jsform, JsRunType::ONLINE);
}

WebViewInterface* DesktopWindow::getWebViewInterface() const
{
    return m_webViewInterface;
}
