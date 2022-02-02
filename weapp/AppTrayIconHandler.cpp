#include "AppTrayIconHandler.h"

#include <QMenu>
#include <QApplication>

AppTrayIconHandler::AppTrayIconHandler(QObject *parent)
    : QObject(parent)
{
    m_menu = new QMenu;
    m_startStopAction   = m_menu->addAction("Start Working");
    m_showTimerAction   = m_menu->addAction("Show Timer");
    m_menu->addSeparator();
    m_dashboardAction   = m_menu->addAction("Open the dashboard");
    m_helpAction        = m_menu->addAction("Help Center");
    m_menu->addSeparator();
    m_aboutAction       = m_menu->addAction("About WEAPP");
    m_preferencesAction = m_menu->addAction("Preferences");
    m_menu->addSeparator();
    m_quitAction        = m_menu->addAction("&Quit");
    enableActionsAndIconTime(false);

    createConnections();
}

AppTrayIconHandler::~AppTrayIconHandler()
{
    m_menu->deleteLater();
}

void AppTrayIconHandler::enableActionsAndIconTime(bool enable)
{
    m_startStopAction->setEnabled(enable);
    m_preferencesAction->setEnabled(enable);
}

void AppTrayIconHandler::startIconTimer(bool start)
{

}

void AppTrayIconHandler::setIconTrackingTime(const QString &time)
{

}

void AppTrayIconHandler::createConnections()
{
    connect(m_startStopAction,   &QAction::triggered, this, &AppTrayIconHandler::actionTriggeredSlot);
    connect(m_showTimerAction,   &QAction::triggered, this, [this](){emit iconActivatedSignal(QSystemTrayIcon::ActivationReason::Trigger);});
    connect(m_dashboardAction,   &QAction::triggered, this, &AppTrayIconHandler::actionTriggeredSlot);
    connect(m_helpAction,        &QAction::triggered, this, &AppTrayIconHandler::actionTriggeredSlot);
    connect(m_aboutAction,       &QAction::triggered, this, &AppTrayIconHandler::actionTriggeredSlot);
    connect(m_preferencesAction, &QAction::triggered, this, &AppTrayIconHandler::actionTriggeredSlot);
    connect(m_quitAction,        &QAction::triggered, qApp, &QApplication::quit);
}

bool AppTrayIconHandler::eventFilter(QObject *watched, QEvent *event)
{
    return false;
}

void AppTrayIconHandler::actionTriggeredSlot()
{
    QAction* sender = dynamic_cast<QAction *>(QObject::sender());
    emit actionTriggeredSignal(sender->text());
}

void AppTrayIconHandler::setWorkingActionText(const QString& text) const
{
    m_startStopAction->setText(text);
}



