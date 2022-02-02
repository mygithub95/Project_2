#ifndef CONTROLTRACKER_H
#define CONTROLTRACKER_H

#include "Info.h"

#include <QObject>

class TrackingManager;

class ControlTracker : public QObject
{
    Q_OBJECT
public:
    explicit ControlTracker(QObject *parent = nullptr);
    ~ControlTracker();
    QString getStorageBucketNames(const QString& credentials);

private:
    void createMembers();
    void makeConnections();

    void showErrorMessage(const QString msg);

    TrackingManager* m_trackingManager;

signals:
    void signOutRequestFinished();

public slots:
    void onLogOutSlot();
    void startOrStopTrackerSlot(const QString& message);
    void credentialsReceivedSlot(const QString& credentials);
    void bucketNameReceivedSlot(const QString& bucketName);

private slots:
    void onSettingsGetRequestFinished(); //Request succeded
    void logErrorMessage(const QString msg);
    void resetPage();
};

#endif // CONTROLTRACKER_H
