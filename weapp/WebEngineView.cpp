#include "WebEngineView.h"

#include "webview.h"
#include "webpage.h"
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QHBoxLayout>

#include <QWidget>

#define SIGNIN_URL "https://internal.weapp.io/src/modules/auth/sign-in.html"
//#define SIGNIN_URL "https://beta.weapp.io/src/modules/auth/sign-in.html"

WebEngineView::WebEngineView(QWidget* parent)
    : WebViewInterface(parent)
{
    createMembers();
    makeConnections();
    setupLayout();
}

void WebEngineView::createMembers()
{
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);

    m_webView = new WebView(this);
    m_webPage = new WebPage(QWebEngineProfile::defaultProfile(), m_webView);
    m_webView->setPage(m_webPage);
    m_webView->setUrl(QUrl(QStringLiteral(SIGNIN_URL)));
}

void WebEngineView::makeConnections()
{
    connect(m_webPage, &WebPage::checkToken, this, &WebViewInterface::tokenReceived);
    connect(m_webPage->profile(), &QWebEngineProfile::downloadRequested, this, [this](QWebEngineDownloadItem *download) {download->accept();});
    connect(m_webPage,&WebPage::onButtonStartOrStopPressed, this, &WebViewInterface::onButtonStartOrStopPressed);
    connect(m_webPage,&WebPage::updateButtonPressed, this, &WebViewInterface::updateButtonPressed);
    connect(m_webPage,&WebPage::credentialsUploaded, this, &WebViewInterface::credentialsUploaded);
    connect(m_webPage,&WebPage::credentialsTriggered, this, &WebViewInterface::credentialsTriggered);
    connect(m_webPage,&WebPage::bucketNameTriggered, this, &WebViewInterface::bucketNameTriggered);
    connect(m_webPage,&WebPage::notificationTriggered, this, &WebViewInterface::notificationTriggered);
    connect(m_webPage,&WebPage::onAutoStartSignal, this, &WebViewInterface::onAutoStartSignal);
    connect(m_webView, &WebView::devToolsRequested, this, &WebEngineView::handleDevToolsRequested);
}

void WebEngineView::setupLayout()
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_webView);
    this->setLayout(mainLayout);
}

void WebEngineView::runJavaScript(const QString& js, JsRunType type)
{
    type == JsRunType::ONLINE ?
                m_webPage->runJavaScript(js, [this](const QVariant &result){ emit connectionChecked(result.toBool());
    }) :
                m_webPage->runJavaScript(js);
}

WebView *WebEngineView::getWebView()
{
    return m_webView;
}

void WebEngineView::handleDevToolsRequested(QWebEnginePage *source)
{
    auto inspectWindow = new WebEngineView;
    inspectWindow->resize(this->size());
    inspectWindow->show();
    source->setDevToolsPage(inspectWindow->getWebView()->page());
    source->triggerAction(QWebEnginePage::InspectElement);
}

void WebEngineView::closeEvent(QCloseEvent *event)
{
    this->deleteLater();
}

void WebEngineView::loadPage(const QString& url)
{
    if(url.isEmpty())
        return;
    m_webView->setUrl(QUrl(url));
}
