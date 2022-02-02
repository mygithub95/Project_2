#include "SystemTrayHandler.h"
#include "Logger.h"

SystemTrayHandler::SystemTrayHandler(QObject* parent)
    : AppTrayIconHandler(parent)
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_menu);
    m_trayIcon->setIcon(QIcon(":/images/WEAPP-icon.png"));
    m_trayIcon->setToolTip("WEAPP");
    m_trayIcon->show();
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &AppTrayIconHandler::iconActivatedSignal);
}

void SystemTrayHandler::showNotification(const QString &msg)
{
    Logger::instance()->log("Notification received: " + msg);
    QIcon icon(":/images/WEAPP-icon.png");
    m_trayIcon->showMessage("WEAPP", msg, icon, 5000);
}
