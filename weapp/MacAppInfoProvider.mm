#include "MacAppInfoProvider.h"
#include "ImageDownloader.h"

#include <AppKit/NSWorkspace.h>
#include <AppKit/AppKit.h>
#include <QtMac>
#include <QTimer>

#include <QDebug>

namespace
{
    NSString* runAppleScript(NSString* script)
    {
        NSDictionary *errorInfo = nil;
        NSAppleScript *run = [[NSAppleScript alloc] initWithSource:script];
        NSAppleEventDescriptor *theDescriptor = [run executeAndReturnError:&errorInfo];
        NSString *theResult = [theDescriptor stringValue];
        //NSLog(@"%@",theResult);
        return theResult;
    }
}

MacAppInfoProvider::MacAppInfoProvider(QObject *parent)
    :AppInfoProvider(parent)
{
    m_browserInfoTimer = new QTimer(this);
    m_browserInfoTimer->setInterval(5000);
    m_timerForGettingActiveWindow = new QTimer(this);
    m_timerForGettingActiveWindow->setInterval(1000);
    connect(m_timerForGettingActiveWindow, &QTimer::timeout, this, &MacAppInfoProvider::gatherInfoOfActiveWindowSlot);
    connect(m_browserInfoTimer, &QTimer::timeout, this, &MacAppInfoProvider::onBrowserInfoTimerTimeOutSlot);
}

void MacAppInfoProvider::start()
{
    AppInfoProvider::start();
    if (m_isBrowserInfoEnabled) {
        m_browserInfoTimer->start();
    }
    if(m_isAppInfoEnabled) {
        m_activeWindowMap.clear();
        gatherInfoOfActiveWindowSlot();
        m_timerForGettingActiveWindow->start();
    }
}

void MacAppInfoProvider::stop()
{
    AppInfoProvider::stop();
    m_browserInfoTimer->stop();
    if(m_isAppInfoEnabled) {
        m_timerForGettingActiveWindow->stop();
    }
}

std::vector<AppAndBrowserInfo> MacAppInfoProvider::getRunningAppInfo()
{
    std::vector<AppAndBrowserInfo> infoVec;
    addTimeDiffToLastActiveWindow();
    for(auto activeApp : m_activeWindowMap){
        if(activeApp.second.name.isEmpty()) continue;
        activeApp.second.runDuration /= 1000;
        infoVec.push_back(activeApp.second);
    }
    m_activeWindowMap.clear();
    m_lastActiveWindow = "";
    return infoVec;
}


std::vector<AppAndBrowserInfo> MacAppInfoProvider::getRunningBrowserInfo()
{
    if (m_browsersInfo.empty()) {
        updateBrowsersMainInfo();
    }
    return m_browsersInfo;

}

void MacAppInfoProvider::onBrowserInfoTimerTimeOutSlot()
{
    updateBrowsersMainInfo();
    updateBrowsersInfoIcons();
}

void MacAppInfoProvider::updateBrowsersMainInfo()
{
    m_browsersInfo.clear();
    m_browsersInfoURLs.clear();

    bool foundChrome = false;
    bool foundSafari = false;

    NSWorkspace * ws = [NSWorkspace sharedWorkspace];
    NSArray * apps = [ws runningApplications];
    NSUInteger count = [apps count];
    for (NSUInteger i = 0; i < count; i++) {
        NSRunningApplication *app = [apps objectAtIndex: i];

        if(app.activationPolicy == NSApplicationActivationPolicyRegular && app.isFinishedLaunching) {
            if([app.localizedName isEqualToString: @"Google Chrome"]) {
                foundChrome = true;
            }
            else if ([app.localizedName isEqualToString: @"Safari"]) {
                foundSafari = true;
            }
            //NSLog(@"%@",app.localizedName);
        }
    }

    AppAndBrowserInfo info;
    info.type = "WEB";
    if (foundChrome) {
        //    NSString* sourceChromeTitle = @"tell application \"Google Chrome\"\n"
        //            "set theTitle to the title of the active tab of the front window \n"
        //        "end tell";
        NSString* sourceChromeURL = @"tell application \"Google Chrome\"\n"
                                    "set theURL to the URL of the active tab of the front window \n"
                                    "end tell";
        NSString* chromeActiveTabURL = runAppleScript(sourceChromeURL);
        //NSString* chromeActiveTabTitle = runAppleScript(sourceChromeTitle);

        if (chromeActiveTabURL) {
            info.name = getNameFromURL(QString::fromNSString(chromeActiveTabURL));
            m_browsersInfo.push_back(info);
            m_browsersInfoURLs.push_back(QString::fromNSString(chromeActiveTabURL));
        }
    }

    if (foundSafari) {
        //NSString* sourceSafariTitle = @"tell application \"Safari\"\n"
        //           "set theTitle to name of current tab of window 1 \n"
        //       "end tell";

        NSString* sourceSafariURL = @"tell application \"Safari\"\n"
                                    "set theURL to URL of current tab of window 1 \n"
                                    "end tell";


        //NSString* safariActiveTabTitle = runAppleScript(sourceSafariTitle);

        NSString* safariActiveTabURL = runAppleScript(sourceSafariURL);


        if (safariActiveTabURL) {
            info.name = getNameFromURL(QString::fromNSString(safariActiveTabURL));
            m_browsersInfo.push_back(info);
            m_browsersInfoURLs.push_back(QString::fromNSString(safariActiveTabURL));
        }
    }
    for (auto& info : m_browsersInfo) {
        qDebug() << info.name;
    }

}

void MacAppInfoProvider::updateBrowsersInfoIcons()
{
    std::vector<QString> urls;
    for (size_t i = 0; i < m_browsersInfo.size(); ++i) {
        if (m_browsersInfo.at(i).icon.isNull()) {
            urls.push_back(m_browsersInfoURLs.at(i));
        }
    }
    std::sort(urls.begin(), urls.end());
    std::unique(urls.begin(), urls.end());

    for (const auto& url : urls) {
        ImageDownloader* imageDownloader = new ImageDownloader(3000);
        connect(imageDownloader, &ImageDownloader::done, this, &MacAppInfoProvider::onURLIconDownloadedSlot);
        imageDownloader->setUrl(url, ImageType::eFavIcon);
    }
}

QString MacAppInfoProvider::getNameFromURL(const QString &url)
{
    QString name(url);
    if (name.startsWith("https://")) {
        name = name.remove(0, 8);
    }
    if (name.startsWith("http://")) {
        name = name.remove(0, 7);
    }
    if (name.startsWith("ftp://")) {
        name = name.remove(0, 6);
    }
    if (name.startsWith("sftp://")) {
        name = name.remove(0, 7);
    }
    if (name.startsWith("www.")) {
        name = name.remove(0, 4);
    }
    int separatorIndex = name.indexOf('/');
    if (separatorIndex >= 0) {
        name = name.left(separatorIndex);
    }
    return name;
}

void MacAppInfoProvider::onURLIconDownloadedSlot(QPixmap icon)
{
    QString url = dynamic_cast<ImageDownloader*>(sender())->getUrl();
    for (size_t i = 0; i < m_browsersInfoURLs.size(); ++i) {
        if (url == m_browsersInfoURLs.at(i)) {
            m_browsersInfo.at(i).icon = icon;
            if(m_browsersInfo.at(i).icon.isNull()){
                m_browsersInfo.at(i).icon = QPixmap(":/icon/browser.svg");
            }
        }
    }
    QObject::sender()->deleteLater();
}

void MacAppInfoProvider::gatherInfoOfActiveWindowSlot()
{
    NSWorkspace * ws = [NSWorkspace sharedWorkspace];
    NSArray * apps = [ws runningApplications];

    NSUInteger count = [apps count];
    for (NSUInteger i = 0; i < count; i++) {
        NSRunningApplication *app = [apps objectAtIndex: i];
        if(!app) continue;
        if (!app.isActive) continue;
        if(app.activationPolicy == NSApplicationActivationPolicyRegular) {

            AppAndBrowserInfo info;
            info.type = "APP";
            info.name = QString::fromNSString(app.localizedName);
            if(info.name != m_lastActiveWindow){
                NSRect imageRect = NSMakeRect(0, 0, app.icon.size.width, app.icon.size.height);
                CGImageRef cgImage = [app.icon CGImageForProposedRect:&imageRect context:NULL hints:nil];
                QPixmap pixmap = QtMac::fromCGImageRef(cgImage);
                info.icon = pixmap;
            }

            if(!m_activeWindowMap.empty()){
                if(info.name != m_lastActiveWindow){
                    addTimeDiffToLastActiveWindow();
                    if(m_activeWindowMap.count(info.name) == 0){
                        m_activeWindowMap[info.name] = info;
                    }
                }
            }else{
                m_startTimeOfActiveWindow = QDateTime::currentDateTime().toUTC();
                m_activeWindowMap[info.name] = info;
            }
            m_lastActiveWindow = info.name;
        }
    }
}

void MacAppInfoProvider::addTimeDiffToLastActiveWindow()
{
    QDateTime now = QDateTime::currentDateTime().toUTC();
    m_activeWindowMap[m_lastActiveWindow].runDuration += m_startTimeOfActiveWindow.msecsTo(now);
    m_startTimeOfActiveWindow = now;
}
