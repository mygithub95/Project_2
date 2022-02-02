#include "webview.h"
#include "webpage.h"
#include "Logger.h"

#include <QMessageBox>
#include <QTimer>
#include <QDesktopServices>
#include <QWebEngineProfile>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSettings>

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)

{
    connect(this, &QWebEngineView::renderProcessTerminated,
            [this](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {
        QString status;
        switch (termStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = tr("Render process normal exit");
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = tr("Render process abnormal exit");
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = tr("Render process crashed");
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = tr("Render process killed");
            break;
        }
        QMessageBox::StandardButton btn = QMessageBox::question(window(), status,
                                                   tr("Render process exited with code: %1\n"
                                                      "Do you want to reload the page ?").arg(statusCode));
        if (btn == QMessageBox::Yes)
            QTimer::singleShot(0, [this] { reload(); });
    });
}

void WebView::setPage(WebPage *page)
{
    QWebEngineView::setPage(page);
}

QWebEngineView *WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    switch (type) {
    case QWebEnginePage::WebBrowserTab: {
        WebView *webView = new WebView;
        WebPage *webPage = new WebPage(this->page()->profile(), webView);
        webView->setPage(webPage);

        connect(webView, &QWebEngineView::urlChanged, this, [this](const QUrl &url) {
            QDesktopServices::openUrl(url);
            sender()->deleteLater();
        });
        return webView;
    }
        return nullptr;
    }
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = page()->createStandardContextMenu();
    menu->removeAction(page()->action(QWebEnginePage::ViewSource));
    menu->removeAction(page()->action(QWebEnginePage::SavePage));
    const QList<QAction *> actions = menu->actions();

    if(checkSettingsFile()){
        auto inspectElement = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::InspectElement));
        if (inspectElement == actions.cend()) {

            auto reload = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::Reload));
            if (reload == actions.cend())
                menu->addSeparator();

            QAction *action = new QAction(menu);
            action->setText("Open inspector in new window");
            connect(action, &QAction::triggered, [this]() {
                emit devToolsRequested(page());
            });

            QAction *before(inspectElement == actions.cend() ? nullptr : *inspectElement);
          menu->addAction(action);
        } else {
            (*inspectElement)->setText(tr("Inspect element"));
        }
    }
    menu->popup(event->globalPos());
}

bool WebView::checkSettingsFile()
{
    QString locationDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString filePath = locationDirPath + "/WEAPP/settings.ini";
    if(!QFile::exists(filePath)){
        Logger::instance()->log("WebView::checkSettingsFile: File doesn't exist.");
        return false;
    }
    QSettings settings(filePath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.beginGroup("inspect");
    bool result = settings.value("EnableInspect").toBool();
    Logger::instance()->log("WebView::checkSettingsFile: File exists. EnableInspect == " + QString::number(result));
    return result;
}
