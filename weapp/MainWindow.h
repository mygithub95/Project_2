#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QSystemTrayIcon>

class DesktopWindow;
class QStackedWidget;
class QLabel;
class FrameWidget;
class ControlTracker;
class AppUpdateController;
class MacDockHandler;
class AppTrayIconHandler;

class MainWindow : public QWidget
{
    Q_OBJECT;
public:
    MainWindow(QWidget *parent = nullptr);

    AppTrayIconHandler* getAppTrayIconHandler();

private:
    void createMembers();
    void makeConnections();
    void setupLayout();
    void installStylesheets();
    void enableActionsAndIconTime(bool enable);

    QStackedWidget* m_stackedWidget;
    DesktopWindow* m_desktopWindow;
    QWidget* m_noInternetConnectionWidget;
    ControlTracker* m_controlTracker;
    AppUpdateController* m_appUpdateController;
    AppTrayIconHandler* m_appTrayIconHandler;


    QLabel* m_labelText;
    FrameWidget* m_labelImage;
    QSystemTrayIcon *m_trayIcon;
    QAction* m_startStopAction;
    QAction* m_showTimerAction;
    QAction* m_dashboardAction;
    QAction* m_helpAction;
    QAction* m_aboutAction;
    QAction* m_preferencesAction;
    QAction* m_quitAction;


private slots:
    void takeTokenAndSignInSlot(const QString& token);
    void credentialsUploadedSlot(const QString& credentials);
    void iconActivatedSlot(QSystemTrayIcon::ActivationReason reason);
    void startOrStopTrackerSlot(const QString& message);
    void onStartStopActionSlot(const QString& actionText);
    void onActionTriggeredSlot(const QString& actionText);
    void runJsToGetTimeForIcon();
    void onAutoStartSlot(bool on);
};

#endif // MAINWINDOW_H
