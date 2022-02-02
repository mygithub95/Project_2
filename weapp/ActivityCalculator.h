#ifndef ACTIVITYCALCULATOR_H
#define ACTIVITYCALCULATOR_H

class QTimer;
class QString;
#include <QObject>

#define ACTIVITY_PERIOD 60 //sec

class ActivityCalculator : public QObject
{
    Q_OBJECT
public:
    ActivityCalculator(QObject* parent = nullptr);
    void startActivityCalculation();
    void stopActivityCalculation();
    ~ActivityCalculator();

private:
    QTimer* m_roundedTimePickerTimer;
    QTimer* m_startCalculationTimer;
    int m_tillRounded;
    bool m_isRunning;

public slots:
    void calculateActivity(int period);
signals:
    void activityIsReady(const QString& percentageOfActivity);
};

#endif // ACTIVITYCALCULATOR_H
