#ifndef WEBENGINEVIEW_H
#define WEBENGINEVIEW_H

#include "WebViewInterface.h"

class WebView;
class WebPage;
class QWebEnginePage;

class WebEngineView : public WebViewInterface
{
public:
    WebEngineView(QWidget* parent = nullptr);

    void runJavaScript(const QString& js, JsRunType type = JsRunType::DEFAULT) override;
    WebView* getWebView();
    void loadPage(const QString& url ="") override;

private:
    void createMembers();
    void makeConnections();
    void setupLayout();

    WebView *m_webView;
    WebPage *m_webPage;

private slots:
    void handleDevToolsRequested(QWebEnginePage *source);

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // WEBENGINEVIEW_H
