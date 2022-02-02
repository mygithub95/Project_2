#ifndef SYSTEMTRAYHANDLER_H
#define SYSTEMTRAYHANDLER_H

#include "AppTrayIconHandler.h"

class SystemTrayHandler : public AppTrayIconHandler
{
    Q_OBJECT
public:
    SystemTrayHandler(QObject* parent = nullptr);
    void showNotification(const QString& msg) override;

private:
    QSystemTrayIcon* m_trayIcon;
};

#endif // SYSTEMTRAYHANDLER_H
