#ifndef WEBVIEWINTERFACE_H
#define WEBVIEWINTERFACE_H

#include <QWidget>

enum JsRunType{
    DEFAULT =0, TOKEN, ONLINE, UPLOAD, CREDENTIALS, BUCKETNAME, GETTIME
};

class WebViewInterface : public QWidget
{
    Q_OBJECT
public:
    WebViewInterface(QWidget *parent = nullptr);

    virtual void runJavaScript(const QString& js, JsRunType type = JsRunType::DEFAULT) = 0;
    virtual void loadPage(const QString& url = "") = 0;

signals:
    void tokenReceived(QString token);
    void onButtonStartOrStopPressed(const QString& message);
    void updateButtonPressed();
    void connectionChecked(bool isOnline);
    void credentialsTriggered(const QString& credentials);
    void bucketNameTriggered(const QString& bucketName);
    void notificationTriggered(const QString& message);
    void credentialsUploaded(const QString& credentials);
    void trackingTimeObtained(const QString& time);
    void onAutoStartSignal(bool on);
};

#endif // WEBVIEWINTERFACE_H
