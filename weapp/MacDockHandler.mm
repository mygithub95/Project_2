#include "MacDockHandler.h"
#include "Logger.h"

#include <QApplicationStateChangeEvent>
#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QTimer>

#import <Foundation/Foundation.h>
#import <Webkit/Webkit.h>

NSStatusItem* m_statusItem;

MacDockHandler::MacDockHandler(QObject* parent)
    : AppTrayIconHandler(parent)
    , m_textTime("00:00:00")
{
    int length = 100;
    m_statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:(CGFloat)length];
    m_menuBarHeight = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];
    Logger::instance()->log("Menu bar height is == " + QString::number(m_menuBarHeight));

    m_timeMidSize = QSize(0, 0);
    m_whiteLogo = QPixmap(":/images/WEAPP-white.png");
    m_whiteLogo = m_whiteLogo.scaled(QSize(m_menuBarHeight-1, m_menuBarHeight-1), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    setMenu(m_menu);
    setIcon(createPixmap(m_textTime));

    m_iconTimer = new QTimer(this);
    m_iconTimer->setInterval(1000);
    connect(m_iconTimer, &QTimer::timeout, this, &MacDockHandler::updateIconTime);
}

void MacDockHandler::enableActionsAndIconTime(bool enable)
{
    AppTrayIconHandler::enableActionsAndIconTime(enable);
    if(!enable)
        setIconTrackingTime("00:00:00");
}

bool MacDockHandler::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == qApp && event->type() == QEvent::ApplicationStateChange) {
        auto ev = static_cast<QApplicationStateChangeEvent*>(event);
        if (m_prevAppState == Qt::ApplicationActive
                && ev->applicationState() == Qt::ApplicationActive) {
            emit iconActivatedSignal(QSystemTrayIcon::Trigger);
        }
        m_prevAppState = ev->applicationState();
    }
    return false;
}

void MacDockHandler::setMenu(QMenu* menu)
{
    NSMenu * nsMenu = menu->toNSMenu();
    [m_statusItem setMenu:nsMenu];
}

void MacDockHandler::setIcon(const QPixmap& pixmap)
{
    QImage image = pixmap.toImage();
    CGImageRef cgImage = image.toCGImage();
    NSImage *nsImage = [[NSImage alloc] initWithCGImage:cgImage size:NSZeroSize];
    nsImage.size = (pixmap.size() / pixmap.devicePixelRatioF()).toCGSize();
    CGImageRelease(cgImage);

    [m_statusItem setImage:nsImage];
    [nsImage release];
}

void MacDockHandler::showNotification(const QString& msg)
{
    Logger::instance()->log("MacDockHandler::showNotification: Notification received: " + msg);
    NSString *notifyText = msg.toNSString();

    id userNotification = [[NSClassFromString(@"NSUserNotification") alloc] init];
    [userNotification performSelector:@selector(setTitle:) withObject:@"WEAPP"];
    [userNotification performSelector:@selector(setInformativeText:) withObject:notifyText];

    id notificationCenterInstance = [NSClassFromString(@"NSUserNotificationCenter") performSelector:@selector(defaultUserNotificationCenter)];
    [notificationCenterInstance performSelector:@selector(deliverNotification:) withObject:userNotification];

    [userNotification release];
}

void MacDockHandler::setIconTrackingTime(const QString &time)
{
    m_textTime = time;
    setIcon(createPixmap(m_textTime));
}

void MacDockHandler::updateIconTime()
{
    inctrementTime();
    setIcon(createPixmap(m_textTime));
}

QSize MacDockHandler::getSizeForText(const QString& text) const
{
    QRect textRect;

    QFontMetrics fm(QFont("Segoe UI", 12, QFont::Bold));
    textRect = fm.tightBoundingRect(text);
    textRect.setHeight(fm.tightBoundingRect("0123456789").height());
    return textRect.size();
}

QPixmap MacDockHandler::createPixmap(const QString& text)
{
    QPen pen;
    pen.setColor(QColor("white"));

    QSize textSize = getSizeForText(text);
    QPixmap textPixmap(textSize);
    textPixmap.fill(QColor(0 ,0, 0));

    QPainter painter1(&textPixmap);
    painter1.setPen(pen);
    painter1.setFont(QFont("Segoe UI", 12, QFont::Bold));
    painter1.drawText(textPixmap.rect(), Qt::AlignCenter, text);

    if(m_timeMidSize.isNull())
        m_timeMidSize = textSize;

    QPixmap pixmap(QSize(100, m_menuBarHeight-1));
    pixmap.fill(QColor(0 ,0, 0));

    QPainter painter(&pixmap);
    painter.drawPixmap(QRect(0,0,m_menuBarHeight-1, m_menuBarHeight -1), m_whiteLogo);
    painter.drawPixmap(QPoint(m_menuBarHeight-1 + (100 - m_menuBarHeight-1)/2 - m_timeMidSize.width()/2, (m_menuBarHeight-1)/2 - m_timeMidSize.height()/2), textPixmap);

    return pixmap;
}

void MacDockHandler::startIconTimer(bool start)
{
    start ? m_iconTimer->start() : m_iconTimer->stop();
}

void MacDockHandler::inctrementTime()
{
    QStringList list = m_textTime.split(':');
    int hour = list[0].toInt();
    int minute = list[1].toInt();
    int second = list[2].toInt();

    second ++;
    if(second == 60)
    {
        minute++;
        second = 0;
    }
    if(minute == 60)
    {
        hour++;
        minute = 0;
    }
    list[0] = hour < 10 ? "0" + QString::number(hour) : QString::number(hour);
    list[1] = minute < 10 ? "0" + QString::number(minute) : QString::number(minute);
    list[2] = second < 10 ? "0" + QString::number(second) : QString::number(second);
    m_textTime = list.join(":");
}
