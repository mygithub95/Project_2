#ifndef MACWEBVIEW_H
#define MACWEBVIEW_H

#include "WebViewInterface.h"

class QMacCocoaViewContainer;

class MacWebView : public WebViewInterface
{
public:
    MacWebView(QWidget* parent = nullptr);

    void runJavaScript(const QString& js, JsRunType type = JsRunType::DEFAULT) override;
    void onConsoleMessage(const QString& message);
    void loadPage(const QString& url = "") override;

private:
    void setupLayout();
    void requestForCloudCredentialsAndBucketName();

    QMacCocoaViewContainer* m_cocoaView;
    QString m_token;
};

#endif // MACWEBVIEW_H
