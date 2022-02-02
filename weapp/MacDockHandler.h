#ifndef MACDOCKHANDLER_H
#define MACDOCKHANDLER_H

#include "AppTrayIconHandler.h"

class QTimer;

class MacDockHandler : public AppTrayIconHandler
{
    Q_OBJECT
public:
    MacDockHandler(QObject* parent = nullptr);

    void enableActionsAndIconTime(bool enable) override;
    void startIconTimer(bool start) override;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    Qt::ApplicationState m_prevAppState = Qt::ApplicationInactive;

private:
    void setMenu(QMenu* menu);
    void setIcon(const QPixmap& pixmap);

    QSize getSizeForText(const QString& text) const ;
    QPixmap createPixmap(const QString& text);
    void inctrementTime();

    int m_menuBarHeight;
    QString m_textTime;
    QSize m_timeMidSize;
    QPixmap m_whiteLogo;
    QTimer* m_iconTimer;

public slots:
    void setIconTrackingTime(const QString& time) override;
    void showNotification(const QString& msg) override;

private slots:
    void updateIconTime();
};

#endif // MACDOCKHANDLER_H
