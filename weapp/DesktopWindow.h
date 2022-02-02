#ifndef DESKTOPWINDOW_H
#define DESKTOPWINDOW_H

#include <QWidget>

class QPushButton;
class QLabel;
class QStackedWidget;
class FrameWidget;

class WebViewInterface;

class DesktopWindow : public QWidget
{
    Q_OBJECT

public:
    DesktopWindow(QWidget *parent = nullptr);
    ~DesktopWindow();

    WebViewInterface* getWebViewInterface() const;

private:
    void createMembers();
    void makeConnections();
    void setupLayout();

    QWidget* m_widgetInternetConnection;

    QPushButton* m_buttonStartAndStop;
    WebViewInterface* m_webViewInterface;

signals:
    void tokenIsReady(const QString& token);
    void internetConectionIsChecked(bool result);
    void startOrStopTrackerSignal(const QString& message);
    void updateApplicationSignal();
    void credentialsReceivedSignal(const QString& credentials);
    void bucketNameReceivedSignal(const QString& bucketName);
    void notificationReceivedSignal(const QString& message);
    void credentialsUploadedSignal(const QString& credentials);
    void trackingTimeObtainedSignal(const QString& time);
    void onAutoStartSignal(bool on);

private slots:
    void onTimerTimoutSlot();
};
#endif // MAINWINDOW_H
