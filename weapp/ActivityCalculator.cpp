#include "ActivityCalculator.h"
#include "ActiveEventsTimeProvider.h"
#include "CustomTimer.h"
#include <QDateTime>
#include <QTimer>
#include <QString>

ActivityCalculator::ActivityCalculator(QObject *parent)
    : QObject(parent)
{
    m_roundedTimePickerTimer = new QTimer(this);
    m_startCalculationTimer = new QTimer(this);
    m_roundedTimePickerTimer->setSingleShot(true);
    connect(m_roundedTimePickerTimer, &QTimer::timeout, this, [this] () {
        calculateActivity(m_tillRounded/1000);
        m_startCalculationTimer->start(ACTIVITY_PERIOD * 1000);
    });
    connect(m_startCalculationTimer, &QTimer::timeout, this, [this](){calculateActivity(ACTIVITY_PERIOD);} );
}

void ActivityCalculator::startActivityCalculation()
{
    QDateTime startTime = QDateTime::currentDateTime();
    m_tillRounded = CustomTimer::getMsecTillTick(startTime.time());
    m_isRunning = ActiveEventsTimeProvider::instance()->startEventsRegistration();
    if(m_isRunning){
        m_roundedTimePickerTimer->start(m_tillRounded);
    }
}

void ActivityCalculator::stopActivityCalculation()
{
    if(m_isRunning){
        ActiveEventsTimeProvider::instance()->stopEventsRegistration();
        m_roundedTimePickerTimer->stop();
        m_startCalculationTimer->stop();
    }
}

ActivityCalculator::~ActivityCalculator()
{}

void ActivityCalculator::calculateActivity(int period)
{
    int activSeconds = ActiveEventsTimeProvider::instance()->getActiveSeconds();
    ActiveEventsTimeProvider::instance()->resetActiveSeconds();
    int activity = qRound((double)activSeconds/period >1 ? 100 : (double)activSeconds/period * 100);
    emit activityIsReady(QString::number(activity));
}
