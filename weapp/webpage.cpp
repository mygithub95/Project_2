#include "webpage.h"
#include "Logger.h"

#include <QAuthenticator>
#include <QMessageBox>
#include <QWebEngineCertificateError>

WebPage::WebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
    , m_token("")
{
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &WebPage::handleFeaturePermissionRequested);
    connect(this, &QWebEnginePage::registerProtocolHandlerRequested, this, &WebPage::handleRegisterProtocolHandlerRequested);
#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    connect(this, &QWebEnginePage::selectClientCertificate, this, &WebPage::handleSelectClientCertificate);
#endif
}

bool WebPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    QString jsform = "function getToken() {"
                     "var token = window.localStorage.getItem('token'); "
                     "return token;"
                     "}"
                     "getToken()";

    this->runJavaScript(jsform, [this](const QVariant &result){
        QString token = result.toString();
        emit checkToken(token);
        if(!token.isEmpty() && m_token.isEmpty()){
            m_token = token;
            Logger::instance()->log("WebPage::acceptNavigationRequest: run js functions setIsApp() & setIsRunningApp(false)");
            this->runJavaScript("setIsApp()");
            this->runJavaScript("setIsRunningApp(false)");
        }
    });
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

void WebPage::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    if(message == "Start tracking" || message == "Stop tracking"){
        emit onButtonStartOrStopPressed(message);
        if(message == "Start tracking")
            requestForCloudCredentialsAndBucketName();
    }
    else if(message == "update_available"){
        emit updateButtonPressed();
    }
    else if(message == "google_credentials_upload"){
        this->runJavaScript("getUserBucketCredentials()", [this](const QVariant &result){
            QString credentials = result.toString();
            Logger::instance()->log("Emit Google credentials upload");
            emit credentialsUploaded(credentials);
        });
    }
    else if(message == "Google bucket name updated"){
        Logger::instance()->log("Emit Google bucket name updated");
        requestForCloudCredentialsAndBucketName();
    }
    else if(message.startsWith("notification_")){
        Logger::instance()->log("Emit notification received " + message);
        auto list = message.split('_');
        emit notificationTriggered(list[1]);
    }
    else if(message == "autostart on" || message == "autostart off"){
        Logger::instance()->log("Console message for startup " + message);
        emit onAutoStartSignal(message == "autostart on");
    }

}

inline QString questionForFeature(QWebEnginePage::Feature feature)
{
    switch (feature) {
    case QWebEnginePage::Geolocation:
        return WebPage::tr("Allow %1 to access your location information?");
    case QWebEnginePage::MediaAudioCapture:
        return WebPage::tr("Allow %1 to access your microphone?");
    case QWebEnginePage::MediaVideoCapture:
        return WebPage::tr("Allow %1 to access your webcam?");
    case QWebEnginePage::MediaAudioVideoCapture:
        return WebPage::tr("Allow %1 to access your microphone and webcam?");
    case QWebEnginePage::MouseLock:
        return WebPage::tr("Allow %1 to lock your mouse cursor?");
    case QWebEnginePage::DesktopVideoCapture:
        return WebPage::tr("Allow %1 to capture video of your desktop?");
    case QWebEnginePage::DesktopAudioVideoCapture:
        return WebPage::tr("Allow %1 to capture audio and video of your desktop?");
    case QWebEnginePage::Notifications:
        return QString();
    }
    return QString();
}

void WebPage::handleFeaturePermissionRequested(const QUrl &securityOrigin, Feature feature)
{
    QString title = tr("Permission Request");
    QString question = questionForFeature(feature).arg(securityOrigin.host());
    if (!question.isEmpty() && QMessageBox::question(view()->window(), title, question) == QMessageBox::Yes)
        setFeaturePermission(securityOrigin, feature, PermissionGrantedByUser);
    else
        setFeaturePermission(securityOrigin, feature, PermissionDeniedByUser);
}

//! [registerProtocolHandlerRequested]
void WebPage::handleRegisterProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request)
{
    auto answer = QMessageBox::question(
        view()->window(),
        tr("Permission Request"),
        tr("Allow %1 to open all %2 links?")
        .arg(request.origin().host())
        .arg(request.scheme()));
    if (answer == QMessageBox::Yes)
        request.accept();
    else
        request.reject();
}

//! [registerProtocolHandlerRequested]

#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
void WebPage::handleSelectClientCertificate(QWebEngineClientCertificateSelection selection)
{
    // Just select one.
    selection.select(selection.certificates().at(0));
}

void WebPage::requestForCloudCredentialsAndBucketName()
{
    this->runJavaScript("getUserBucketCredentials()", [this](const QVariant &result){
        QString credentials = result.toString();
        emit credentialsTriggered(credentials);
    });
    this->runJavaScript("getUserBucketName()", [this](const QVariant &result){
        QString bucketName = result.toString();
        emit bucketNameTriggered(bucketName);
    });
}
#endif
