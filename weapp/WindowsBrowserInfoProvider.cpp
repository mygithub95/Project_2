#include "WindowsBrowserInfoProvider.h"
#include "ImageDownloader.h"
#include "WinUtilities.h"

#include <AtlBase.h>
#include <AtlCom.h>
#include <UIAutomation.h>

#include <QTimer>

WindowsBrowserInfoProvider::WindowsBrowserInfoProvider(QObject *parent)
    : QObject(parent)
{
    m_timerForDownloadBrowserIcon= new QTimer(this);
    m_timerForDownloadBrowserIcon->setInterval(3000);
    connect(m_timerForDownloadBrowserIcon, &QTimer::timeout, this, &WindowsBrowserInfoProvider::onTimerOutDownloadIconSlot);
}

void WindowsBrowserInfoProvider::start()
{
    m_timerForDownloadBrowserIcon->start();
}

void WindowsBrowserInfoProvider::stop()
{
    m_timerForDownloadBrowserIcon->stop();
    m_browsersInfo.clear();
}

std::vector<AppAndBrowserInfo> WindowsBrowserInfoProvider::getBrowsersInfo()
{
    std::vector<AppAndBrowserInfo> browsersInfo;
    for(const auto& info : m_browsersInfo){
        browsersInfo.push_back(info.second);
    }
    m_browsersInfo.clear();
    return browsersInfo;
}

void WindowsBrowserInfoProvider::onTimerOutDownloadIconSlot()
{
    m_activeTabs.clear();
    m_activeTabs = getActiveTabs(BrowserType::eChrome);
    auto tabsMozilla = getActiveTabs(BrowserType::eMozilla);
    m_activeTabs.insert(tabsMozilla.begin(), tabsMozilla.end());
    auto tabsMicrosoftEdge = getActiveTabs(BrowserType::eMicrosoftEdge);
    m_activeTabs.insert(tabsMicrosoftEdge.begin(), tabsMicrosoftEdge.end());

    for(const auto& browser : m_activeTabs){
        if(!browser.second.isEmpty() && m_browsersInfo[browser.second].icon.isNull())
            creatNewBrowserInfo(browser);
    }
}

void WindowsBrowserInfoProvider::creatNewBrowserInfo(const std::pair<const QString, QString> tabinfo)
{
    m_browsersInfo[tabinfo.second].name = tabinfo.second;
    m_browsersInfo[tabinfo.second].type = "WEB";
    ImageDownloader *imageDownloader = new ImageDownloader(3000);
    connect(imageDownloader, &ImageDownloader::done, this, [=](QPixmap pix){
        if(pix.isNull()){
            pix = QPixmap(":/icon/browser.svg");
        }
        pix = pix.scaled(32, 32, Qt::AspectRatioMode::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_browsersInfo[m_activeTabs[tabinfo.first]].icon  = pix;
        imageDownloader->deleteLater();
    });
    imageDownloader->setUrl(tabinfo.first, ImageType::eFavIcon);
}

std::map<QString, QString> WindowsBrowserInfoProvider::getActiveTabs(BrowserType type)
{
    std::map<QString, QString> activeTab;

    CoInitialize(NULL);
    HWND hwnd = NULL;
    while (true)
    {
        if(type == BrowserType::eMozilla){
            hwnd = FindWindowEx(0, hwnd, L"MozillaWindowClass", NULL);
        }
        else if (type == BrowserType::eChrome || type == BrowserType::eMicrosoftEdge){
            hwnd = FindWindowEx(0, hwnd, L"Chrome_WidgetWin_1", NULL);
        }

        if (!hwnd){
            break;
        }

        DWORD dwProcId = 0;
        GetWindowThreadProcessId(hwnd, &dwProcId);
        QString exeName = getProcessExeNameByID(dwProcId).toLower();
        if(exeName.endsWith("skype.exe"))
            continue;

        if (!IsWindowVisible(hwnd) )
            continue;

        CComQIPtr<IUIAutomation> uia;
        if (FAILED(uia.CoCreateInstance(CLSID_CUIAutomation)) || !uia){
            break;
        }
        CComPtr<IUIAutomationElement> root;
        if (FAILED(uia->ElementFromHandle(hwnd, &root)) || !root){
            break;
        }

        QString urlStr, browserName;
        if(type == BrowserType::eMozilla){
            CComPtr<IUIAutomationCondition> conditionNavigation;
            uia->CreatePropertyCondition(UIA_NamePropertyId, CComVariant(L"Navigation"), &conditionNavigation);

            CComPtr<IUIAutomationElement> navigationElement;
            if (FAILED(root->FindFirst(TreeScope_Descendants, conditionNavigation, &navigationElement)) || !navigationElement)
                continue;

            CComVariant toolBar;
            navigationElement->GetCurrentPropertyValue(UIA_LocalizedControlTypePropertyId, &toolBar);
            QString toolBarStr = QString::fromWCharArray(toolBar.tagVARIANT::bstrVal);
            if(toolBarStr != "tool bar"){
                continue;
            }

            CComPtr<IUIAutomationCondition> conditionComboBox;
            uia->CreatePropertyCondition(UIA_LocalizedControlTypePropertyId, CComVariant(L"combo box"), &conditionComboBox);

            CComPtr<IUIAutomationElement> comboBoxElement;
            if (FAILED(navigationElement->FindFirst(TreeScope_Descendants, conditionComboBox, &comboBoxElement)) || !comboBoxElement)
                continue;

            CComPtr<IUIAutomationCondition> conditionEdit;
            uia->CreatePropertyCondition(UIA_LocalizedControlTypePropertyId, CComVariant(L"edit"), &conditionEdit);

            CComPtr<IUIAutomationElement> editElement;
            if (FAILED(comboBoxElement->FindFirst(TreeScope_Descendants, conditionEdit, &editElement)) || !editElement)
                continue;
            CComVariant url;
            editElement->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
            urlStr = QString::fromWCharArray(url.tagVARIANT::bstrVal);
        }
        else if (type == BrowserType::eChrome){
            CComPtr<IUIAutomationCondition> condition;
            uia->CreatePropertyCondition(UIA_NamePropertyId, CComVariant(L"Google Chrome"), &condition);
            CComPtr<IUIAutomationElement> chrome;
            if (FAILED(root->FindFirst(TreeScope_Children, condition, &chrome)) || !chrome){
                continue;
            }

            uia->CreatePropertyCondition(UIA_NamePropertyId, CComVariant(L"Address and search bar"), &condition);
            CComPtr<IUIAutomationElement> edit;
            if (FAILED(chrome->FindFirst(TreeScope_Descendants, condition, &edit)) || !edit){
                continue;
            }

            CComVariant url;
            edit->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
            urlStr = QString::fromWCharArray(url.tagVARIANT::bstrVal);
        }
        else if (type == BrowserType::eMicrosoftEdge){
            CComVariant name;
            root->GetCurrentPropertyValue(UIA_NamePropertyId, &name);
            browserName = QString::fromWCharArray(name.tagVARIANT::bstrVal);
            if(browserName.endsWith("Google Chrome")){
                continue;
            }
            CComPtr<IUIAutomationCondition> condition;
            uia->CreatePropertyCondition(UIA_NamePropertyId, CComVariant(L"Microsoft Edge"), &condition);
            CComPtr<IUIAutomationElement> microsoftEdge;
            if (FAILED(root->FindFirst(TreeScope_Descendants, condition, &microsoftEdge)) || !microsoftEdge){
                continue;
            }

            uia->CreatePropertyCondition(UIA_NamePropertyId, CComVariant(L"Address and search bar"), &condition);
            CComPtr<IUIAutomationElement> edit;
            if (FAILED(microsoftEdge->FindFirst(TreeScope_Descendants, condition, &edit)) || !edit){
                continue;
            }
            CComVariant url;
            edit->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
            urlStr = QString::fromWCharArray(url.tagVARIANT::bstrVal);
        }
        activeTab[urlStr] = getNameFromURL(urlStr);
    }
    CoUninitialize();
    return activeTab;
}

QString WindowsBrowserInfoProvider::getNameFromURL(const QString &url)
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
