#ifndef ACTIVEEVENTSTIMEPROVIDER_H
#define ACTIVEEVENTSTIMEPROVIDER_H
#include <uiohook.h>
#include <QThread>
#include <QVector>
#include <QMutex>

class ActiveEventsTimeProvider : private QThread
{
public:
    static ActiveEventsTimeProvider* instance();

    bool startEventsRegistration();
    void stopEventsRegistration();

    int getActiveSeconds();
    void resetActiveSeconds();

protected:
    void run() override;

private:
    ActiveEventsTimeProvider();
    ~ActiveEventsTimeProvider();
    //remove copy constructor and assignment operator for Singleton
    ActiveEventsTimeProvider(const ActiveEventsTimeProvider&) = delete;
    ActiveEventsTimeProvider& operator=(const ActiveEventsTimeProvider&) = delete;

    void startHookProcess();
    void stopHookProcess();
    static void dispatch_proc(uiohook_event * const event);

    static QMutex m_dataMutex;
    static QVector<int> m_activitySeconds;
};

#endif // ACTIVEEVENTSTIMEPROVIDER_H
