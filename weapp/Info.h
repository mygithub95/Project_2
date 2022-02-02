#ifndef INFO_H
#define INFO_H

#include <QMetaType>
#include <QString>
#include <QPixmap>
#define VERSION 1.24

struct AppAndBrowserInfo
{
    AppAndBrowserInfo()
        : name("")
        , type("")
        , icon(QPixmap())
        , screenId("0")
        , runDuration(0)
    {}

    QString name;
    QString type;
    QPixmap icon;
    QString screenId;
    double runDuration;
};

struct VideoFilesInfo
{
    VideoFilesInfo()
        : videoFileName("")
        , miniVideoFileName("")
        , startTime("")
        , screenId(-1)
    {}

    QString videoFileName;
    QString miniVideoFileName;
    QString startTime;
    int screenId;
};

struct SettingStatesInfo
{
    SettingStatesInfo()
        : permissionEnabled(false)
        , screenRecordEnabled(true)
        , appsEnabled(true)
        , websitesEnabled(true)
    {}

    bool permissionEnabled;
    bool screenRecordEnabled;
    bool appsEnabled;
    bool websitesEnabled;
};

Q_DECLARE_METATYPE(AppAndBrowserInfo);
Q_DECLARE_METATYPE(VideoFilesInfo);
#endif // APPINFO_H
