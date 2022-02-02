#ifndef APPTRAYICONHANDLER_H
#define APPTRAYICONHANDLER_H

#include <QObject>
#include <QSystemTrayIcon>

class QAction;
class QMenu;

class AppTrayIconHandler : public QObject
{
    Q_OBJECT
public:
    explicit AppTrayIconHandler(QObject *parent = nullptr);
    virtual ~AppTrayIconHandler();

    virtual void enableActionsAndIconTime(bool enable);
    virtual void startIconTimer(bool start);

    void setWorkingActionText(const QString& text) const;

private:
    void createConnections();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) ;

    QMenu* m_menu;
    QAction* m_startStopAction;
    QAction* m_showTimerAction;
    QAction* m_dashboardAction;
    QAction* m_helpAction;
    QAction* m_aboutAction;
    QAction* m_preferencesAction;
    QAction* m_quitAction;

public slots:
    virtual void setIconTrackingTime(const QString& time);
    virtual void showNotification(const QString& msg) = 0;

private slots:
    void actionTriggeredSlot();

signals:
    void actionTriggeredSignal(const QString& actionName);
    void iconActivatedSignal(QSystemTrayIcon::ActivationReason reason);
};

#endif // APPTRAYICONHANDLER_H
