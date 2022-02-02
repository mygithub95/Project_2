#include "LinuxAppInfoProvider.h"

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QTextStream>

#include <X11/Xlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#define MAX_PROPERTY_VALUE_LEN 4096

LinuxAppInfoProvider::LinuxAppInfoProvider()
{
    init_charset();
    m_timerForGettingActiveWindow = new QTimer(this);
    m_timerForGettingActiveWindow->setInterval(1000);
    connect(m_timerForGettingActiveWindow, &QTimer::timeout, this, &LinuxAppInfoProvider::gatherInfoOfActiveWindowSlot);
}

LinuxAppInfoProvider::~LinuxAppInfoProvider()
{
    XCloseDisplay(m_disp);
}

void LinuxAppInfoProvider::start()
{
    AppInfoProvider::start();
    if(m_isAppInfoEnabled) {
        m_activeWindowMap.clear();
        gatherInfoOfActiveWindowSlot();
        m_timerForGettingActiveWindow->start();
    }
}

void LinuxAppInfoProvider::stop()
{
    AppInfoProvider::stop();
    if(m_isAppInfoEnabled) {
        m_timerForGettingActiveWindow->stop();
    }
}

QString list_dir(const char *path, QString lastBit) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        return "";
    }

    // removing one simbol to detect related file of running app
    // for example chrome => 'e' to find chromium-browser.desktop file
    if(!lastBit.isEmpty())
        lastBit.truncate(lastBit.size()-1);

    while ((entry = readdir(dir)) != NULL) {
        if(QString(entry->d_name).contains(lastBit) && QString(entry->d_name).contains(".desktop"))
        {
            closedir(dir);
            return QString(entry->d_name);
        }
    }
    closedir(dir);
    return "";
}

std::vector<AppAndBrowserInfo> LinuxAppInfoProvider::getRunningAppInfo()
{
    addTimeDiffToLastActiveWindow();
    std::vector<AppAndBrowserInfo> appInfos;

    for(auto runningAppInfo : m_activeWindowMap) {
        if(runningAppInfo.second.path.isEmpty()){
            continue;
        }

        QStringList parts = runningAppInfo.second.path.split("/");
        QString lastBit = parts.at(parts.size()-1);

        QString hugeFile = "/usr/share/applications/" + list_dir("/usr/share/applications/", lastBit);
        QFile file(hugeFile);
        QString iconFileName;
        if (file.open(QFile::ReadOnly ))
        {
            QTextStream in(&file);
            while (!in.atEnd()) {
                auto line = in.readLine();
                if(line.contains("Icon"))
                {
                    iconFileName = line.split("=").back();
                    break;
                }
            }
        }

        file.close();

        QString iconPath = getIconPath("48x48", iconFileName);
        QPixmap icon = QPixmap(iconPath);
        if(icon.isNull())
        {
            iconPath = getIconPath("256x256", iconFileName);
            icon = QPixmap(iconPath);
        }
        if(icon.isNull())
        {
            iconPath = getIconPath("32x32", iconFileName);
            icon = QPixmap(iconPath);
        }
        if(icon.isNull())
            icon = QPixmap(":/icon/executable.svg");

        icon = icon.scaled(64, 64, Qt::AspectRatioMode::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        QString name = parts.last();

        AppAndBrowserInfo appInfo;
        appInfo.name = name;
        appInfo.icon = icon;
        appInfo.type = "APP";
        appInfo.runDuration = (double)runningAppInfo.second.runTime/1000;
        appInfo.screenId = getScreenIdFromBox(runningAppInfo.second.wind);

        appInfos.push_back(appInfo);
    }
    m_activeWindowMap.clear();
    return appInfos;
}

std::vector<AppAndBrowserInfo> LinuxAppInfoProvider::getRunningBrowserInfo()
{
    if (! (m_disp = XOpenDisplay(NULL))) {
        printf("Cannot open display.\n", stderr);
        return {};
    }

    Window *client_list;
    unsigned long client_list_size;
    int i;
    int max_client_machine_len = 0;

    if ((client_list = get_client_list(&client_list_size)) == NULL) {
        return {};
    }

    /* find the longest client_machine name */
    for (i = 0; i < client_list_size / sizeof(Window); i++) {
        gchar *client_machine;
        if ((client_machine = get_property(client_list[i],
                                           XA_STRING, "WM_CLIENT_MACHINE", NULL))) {
            max_client_machine_len = strlen(client_machine);
        }
        g_free(client_machine);
    }

    std::vector<AppAndBrowserInfo> browsersInfos;
    AppAndBrowserInfo browserInfo;
    /* print the list */
    for (i = 0; i < client_list_size / sizeof(Window); i++)
    {
        gchar *title_utf8 = get_window_title(client_list[i]); /* UTF8 */
        gchar *title_out = get_output_str(title_utf8, TRUE);

        QString appFullName = QString(title_out);
        if(appFullName.contains("Chromium") || appFullName.contains("Mozilla Firefox"))
        {
            QStringList parts = appFullName.split("-");
            if(parts.size() > 1)
            {
                parts.pop_back();
                browserInfo.name = parts.join("-");
            }
            else
            {
                browserInfo.name = parts.front();
            }
            browserInfo.icon = QPixmap(":/icon/browser.svg");
            browserInfo.type = "WEB";
            browsersInfos.push_back(browserInfo);
        }

        g_free(title_utf8);
        g_free(title_out);
    }
    g_free(client_list);

    return browsersInfos;
}

QString LinuxAppInfoProvider::getIconPath(const QString &iconsSize, const QString& iconFileName)
{
    QString iconPath;
    //This is the fliters
    QDir::Filters df = QDir::Dirs;
    QDirIterator::IteratorFlag dff = QDirIterator::Subdirectories;

    QString root = "/usr/share/icons/hicolor/" + iconsSize + "/";
    QDirIterator it(root,df,dff);
    while(it.hasNext())
    {
        QString str = it.next();
        if(fileExists(str + "/" + iconFileName + ".png"))
        {
            iconPath = str + "/" + iconFileName + ".png";
            break;
        }
    }
    return iconPath;
}

QString LinuxAppInfoProvider::getScreenIdFromBox(const QRect& appRect)
{
    QDesktopWidget *widget = QApplication::desktop();
    for (int i = 0; i < widget->screenCount(); i++) {
        const auto& screenRect = widget->screenGeometry(i);
        if(screenRect.contains(appRect.topLeft()))
        {
            return QString::number(i);
        }
    }
    return "0";
}

bool LinuxAppInfoProvider::fileExists(QString path) {
    QFileInfo check_file(path);
    // check if path exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

void LinuxAppInfoProvider::init_charset (void) {
    const gchar *charset; /* unused */
    gchar *lang = getenv("LANG") ? g_ascii_strup(getenv("LANG"), -1) : NULL;
    gchar *lc_ctype = getenv("LC_CTYPE") ? g_ascii_strup(getenv("LC_CTYPE"), -1) : NULL;

    /* this glib function doesn't work on my system ... */
    m_envir_utf8 = g_get_charset(&charset);

    /* ... therefore we will examine the environment variables */
    if (lc_ctype && (strstr(lc_ctype, "UTF8") || strstr(lc_ctype, "UTF-8"))) {
        m_envir_utf8 = TRUE;
    }
    else if (lang && (strstr(lang, "UTF8") || strstr(lang, "UTF-8"))) {
        m_envir_utf8 = TRUE;
    }

    g_free(lang);
    g_free(lc_ctype);
}

gchar* LinuxAppInfoProvider::get_output_str (gchar *str, gboolean is_utf8) {
    gchar *out;

    if (str == NULL) {
        return NULL;
    }

    if (m_envir_utf8) {
        if (is_utf8) {
            out = g_strdup(str);
        }
        else {
            if (! (out = g_locale_to_utf8(str, -1, NULL, NULL, NULL))) {
                out = g_strdup(str);
            }
        }
    }
    else {
        if (is_utf8) {
            if (! (out = g_locale_from_utf8(str, -1, NULL, NULL, NULL))) {
                out = g_strdup(str);
            }
        }
        else {
            out = g_strdup(str);
        }
    }

    return out;
}

gchar* LinuxAppInfoProvider::get_property (Window win, /*{{{*/
                                           Atom xa_prop_type, gchar *prop_name, unsigned long *size) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    gchar *ret;

    xa_prop_name = XInternAtom(m_disp, prop_name, False);

    if (XGetWindowProperty(m_disp, win, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
                           xa_prop_type, &xa_ret_type, &ret_format,
                           &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size = (ret_format / (32 / sizeof(long))) * ret_nitems;
    ret = (gchar*)g_malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

gchar* LinuxAppInfoProvider::get_window_class (Window win) {/*{{{*/
    gchar *class_utf8;
    gchar *wm_class;
    unsigned long size;

    wm_class = get_property(win, XA_STRING, "WM_CLASS", &size);
    if (wm_class) {
        gchar *p_0 = strchr(wm_class, '\0');
        if (wm_class + size - 1 > p_0) {
            *(p_0) = '.';
        }
        class_utf8 = g_locale_to_utf8(wm_class, -1, NULL, NULL, NULL);
    }
    else {
        class_utf8 = NULL;
    }

    g_free(wm_class);

    return class_utf8;
}

gchar* LinuxAppInfoProvider::get_window_title (Window win) {/*{{{*/
    gchar *title_utf8;
    gchar *wm_name;
    gchar *net_wm_name;

    wm_name = get_property(win, XA_STRING, "WM_NAME", NULL);

    net_wm_name = get_property(win,
                               XInternAtom(m_disp, "UTF8_STRING", False), "_NET_WM_NAME", NULL);

    if (net_wm_name) {
        title_utf8 = g_strdup(net_wm_name);
    }
    else {
        if (wm_name) {
            title_utf8 = g_locale_to_utf8(wm_name, -1, NULL, NULL, NULL);
        }
        else {
            title_utf8 = NULL;
        }
    }

    g_free(wm_name);
    g_free(net_wm_name);

    return title_utf8;
}

Window* LinuxAppInfoProvider::get_client_list (unsigned long *size) {
    Window *client_list;

    if ((client_list = (Window *)get_property(DefaultRootWindow(m_disp),
                                              XA_WINDOW, "_NET_ACTIVE_WINDOW", size)) == NULL) {
        if ((client_list = (Window *)get_property(DefaultRootWindow(m_disp),
                                                  XA_CARDINAL, "_WIN_CLIENT_LIST", size)) == NULL) {
            fputs("Cannot get client list properties. \n"
                  "(_NET_CLIENT_LIST or _WIN_CLIENT_LIST)"
                  "\n", stderr);
            return NULL;
        }
    }

    return client_list;
}

int get_exe_for_pid(pid_t pid, char *buf, size_t bufsize) {
    char path[32];
    sprintf(path, "/proc/%d/exe", pid);
    return readlink(path, buf, bufsize);
}

void LinuxAppInfoProvider::gatherInfoOfActiveWindowSlot()
{
    if (! (m_disp = XOpenDisplay(NULL))) {
        printf("Cannot open display.\n", stderr);
        return;
    }

    Window *client_list;
    unsigned long client_list_size;
    int i;
    int max_client_machine_len = 0;
    if ((client_list = get_client_list(&client_list_size)) == NULL) {
        return;
    }

    /* find the longest client_machine name */
    for (i = 0; i < client_list_size / sizeof(Window); i++) {
        gchar *client_machine;
        if ((client_machine = get_property(client_list[i],
                                           XA_STRING, "WM_CLIENT_MACHINE", NULL))) {
            max_client_machine_len = strlen(client_machine);
        }
        g_free(client_machine);
    }

    std::vector<RunningAppInfo> appInfos;
    RunningAppInfo appInfo;
    /* print the list */
    for (i = 0; i < client_list_size / sizeof(Window); i++) {
        gchar *class_out = get_window_class(client_list[i]); /* UTF8 */
        unsigned long *pid = nullptr;
        unsigned long *desktop;
        int x, y, junkx, junky;
        unsigned int wwidth, wheight, bw, depth;
        Window junkroot;

        /* desktop ID */
        if ((desktop = (unsigned long *)get_property(client_list[i],
                                                     XA_CARDINAL, "_NET_WM_DESKTOP", NULL)) == NULL) {
            desktop = (unsigned long *)get_property(client_list[i],
                                                    XA_CARDINAL, "_WIN_WORKSPACE", NULL);
        }

        /* pid */
        pid = (unsigned long *)get_property(client_list[i],
                                            XA_CARDINAL, "_NET_WM_PID", NULL);

        /* geometry */
        XGetGeometry (m_disp, client_list[i], &junkroot, &junkx, &junky,
                      &wwidth, &wheight, &bw, &depth);
        XTranslateCoordinates (m_disp, client_list[i], junkroot, junkx, junky,
                               &x, &y, &junkroot);
        const int buf_size = 512;
        char buf[buf_size];

        int ret =0;

        if(pid)
            ret = get_exe_for_pid(*pid, buf, buf_size);

        if (ret > 0)
        {
            buf[ret] = 0;
            printf("%s\n", buf);
        }
        else{
            perror("readlink");
            continue;
        }

        // filtering out folder as app considering
        if(QString(buf).contains("nautilus"))
            continue;

        appInfo.path = QString(buf);
        appInfo.wind = QRect(x,y,wwidth, wheight);

        if(!m_activeWindowMap.empty()){
            if(appInfo.path != m_lastActiveWindow){
                addTimeDiffToLastActiveWindow();
                if(m_activeWindowMap.count(appInfo.path) == 0)
                    m_activeWindowMap[appInfo.path] = appInfo;
            }
        }else{
            m_startTimeOfActiveWindow = QDateTime::currentDateTime().toUTC();
            m_activeWindowMap[appInfo.path] = appInfo;
        }
        m_lastActiveWindow = appInfo.path;
        appInfos.push_back(appInfo);
        g_free(desktop);
        g_free(class_out);
        g_free(pid);
    }
    g_free(client_list);
}

void LinuxAppInfoProvider::addTimeDiffToLastActiveWindow()
{
    QDateTime now = QDateTime::currentDateTime().toUTC();
    m_activeWindowMap[m_lastActiveWindow].runTime += m_startTimeOfActiveWindow.msecsTo(now);
    m_startTimeOfActiveWindow = now;
}
