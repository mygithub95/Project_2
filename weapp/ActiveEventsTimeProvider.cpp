#include "ActiveEventsTimeProvider.h"
#include <stdarg.h>
#include <stdio.h>
#include <QDateTime>
#ifdef __APPLE__
#include <QMessageBox>
#endif


QMutex ActiveEventsTimeProvider::m_dataMutex;
QVector<int> ActiveEventsTimeProvider::m_activitySeconds = {};

ActiveEventsTimeProvider::ActiveEventsTimeProvider()
{}

ActiveEventsTimeProvider::~ActiveEventsTimeProvider()
{}

ActiveEventsTimeProvider *ActiveEventsTimeProvider::instance()
{
    static ActiveEventsTimeProvider* instance = nullptr;
    if (!instance) {
        instance = new ActiveEventsTimeProvider();
    }
    return instance;
}

bool ActiveEventsTimeProvider::startEventsRegistration()
{
#ifdef __APPLE__
    if (!is_macos_accessibility_enabled()) {
        QMessageBox msg;
        msg.setText("Your program will run without activity calculation. \n"
                    "Please enable accessibility and run tracking again.");
        msg.exec();
        return false;
    }
#endif
    start();
    return true;
}

void ActiveEventsTimeProvider::stopEventsRegistration()
{
    stopHookProcess();
    quit();
    wait();
}

void ActiveEventsTimeProvider::dispatch_proc(uiohook_event * const event){
     auto now = QDateTime::currentDateTime();
     int second = now.time().minute() * 60 + now.time().second();

     m_dataMutex.lock();
     if(!m_activitySeconds.empty()){
         if(m_activitySeconds.back() != second) {
             m_activitySeconds.push_back(second);
         }
     }
     else{
        m_activitySeconds.push_back(second);
     }
     m_dataMutex.unlock();
}

void ActiveEventsTimeProvider::startHookProcess()
{
    hook_set_dispatch_proc(&dispatch_proc);
    hook_run();
}

void ActiveEventsTimeProvider::stopHookProcess()
{
    hook_stop();
    m_activitySeconds.clear();
}

void ActiveEventsTimeProvider::run()
{
    startHookProcess();
    QThread::run();
}


int ActiveEventsTimeProvider::getActiveSeconds()
{
    m_dataMutex.lock();
    int size = m_activitySeconds.size();
    m_dataMutex.unlock();
    return size;
}

void ActiveEventsTimeProvider::resetActiveSeconds()
{
    m_dataMutex.lock();
    m_activitySeconds.clear();
    m_dataMutex.unlock();
}
