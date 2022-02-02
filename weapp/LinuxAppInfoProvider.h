#ifndef LINUXAPPINFOPROVIDER_H
#define LINUXAPPINFOPROVIDER_H

#include "AppInfoProvider.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xmu/WinUtil.h>
#include <glib.h>

#undef Bool
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Status
#undef Unsorted

#include <QDateTime>

struct RunningAppInfo{
    RunningAppInfo()
        : runTime(0)
    {}

    QString path;
    QRect wind;
    int runTime;

    bool operator<(const RunningAppInfo& other) const
    {
        return (this->path < other.path);
    }
    bool operator==(const RunningAppInfo &other)
    {
        return (this->path == other.path);
    }
};

class LinuxAppInfoProvider : public AppInfoProvider
{
public:
    LinuxAppInfoProvider();
    ~LinuxAppInfoProvider();

    void start() override;
    void stop() override;

private:
    std::vector<AppAndBrowserInfo> getRunningAppInfo() override;
    std::vector<AppAndBrowserInfo> getRunningBrowserInfo() override;

    QString getIconPath(const QString &iconsSize, const QString& iconFileName);
    QString getScreenIdFromBox(const QRect& appRect);
    static bool fileExists(QString path);

    void init_charset (void);
    gchar* get_output_str (gchar *str, gboolean is_utf8);
    gchar* get_property (Window win, Atom xa_prop_type, gchar *prop_name, unsigned long *size);
    gchar* get_window_class (Window win);
    gchar* get_window_title (Window win);
    Window* get_client_list (unsigned long *size);

    void addTimeDiffToLastActiveWindow();

private:
    std::map<QString, RunningAppInfo> m_activeWindowMap;
    QTimer* m_timerForGettingActiveWindow;
    QDateTime m_startTimeOfActiveWindow;
    QString m_lastActiveWindow;

    gboolean m_envir_utf8;
    Display* m_disp;

private slots:
    void gatherInfoOfActiveWindowSlot();
};

#endif // LINUXAPPINFOPROVIDER_H
