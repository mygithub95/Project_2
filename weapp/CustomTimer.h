#ifndef TIMERFORSCREENSHOT_H
#define TIMERFORSCREENSHOT_H

#include <QObject>

class QTimer;

class CustomTimer : public QObject
{
    Q_OBJECT
public:
    CustomTimer(QObject* parent = nullptr);

    void start();
    void stop();
    void setTimerEnabled(bool enabled);
    static int getMsecTillTick(const QTime & nowTime);

private:
    QTimer* m_tickTimer;
    QTimer* m_timerForScreenShot;
    bool m_isEnabled;

private slots:
    void onTickTimerTimeoutSlot(bool useThisTick);

signals:
    void startScreenRecord(const QString& startTime);
};

#endif // TIMERFORSCREENSHOT_H
