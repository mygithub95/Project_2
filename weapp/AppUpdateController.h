#ifndef APPUPDATECONTROLLER_H
#define APPUPDATECONTROLLER_H

#include <QObject>

class QTimer;

class AppUpdateController : public QObject
{
    Q_OBJECT
public:
    AppUpdateController(QObject* parent = nullptr);
    ~AppUpdateController() {}
private:
    QTimer* m_timer;
    QByteArray m_data;

signals:
    void updateIsAvailable(QString version);

public slots:
    void checkForUpdates();
    void updateApplicationSlot();
private slots:
    void onCheckerThreadFinished();
    void runToolForCheckingUpdates();
};


#endif // APPUPDATECONTROLLER_H
