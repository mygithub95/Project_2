#include "MacWebView.h"
#include "Logger.h"

#include <QMacCocoaViewContainer>
#include <QHBoxLayout>

#import <Foundation/Foundation.h>
#import <Webkit/Webkit.h>
#include <QDesktopServices>
#include <QUrl>

@interface MacWKView : WKWebView

@end

@interface MacWKView () <WKNavigationDelegate, WKScriptMessageHandler, WKUIDelegate> {
    @public MacWebView* qtView;
}
@end

@implementation MacWKView

- (void)prepareWKWebView {

    self.navigationDelegate = self;
    self.UIDelegate = self;
}

#pragma mark - WKScriptMessageHandler

- (void)userContentController:(WKUserContentController *)userContentController
  didReceiveScriptMessage:(WKScriptMessage *)message {
    if (message.name == @"logHandler") {
        NSString *msg =message.body;
        NSLog(@"callback: %@", msg);
        self->qtView->onConsoleMessage(QString::fromNSString(msg));
    }
}

#pragma mark - WKNavigationDelegate

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
    QString jsform = "function getToken() {"
                     "var token = window.localStorage.getItem('token'); "
                     "return token;"
                     "}"
                     "getToken()";
    self->qtView->runJavaScript(jsform, JsRunType::TOKEN);
}

#pragma mark - WKUIDelegate

 - (void)webView:(WKWebView *)webView
   runOpenPanelWithParameters:(WKOpenPanelParameters *)parameters
   initiatedByFrame:(WKFrameInfo *)frame
   completionHandler:(void (^)(NSArray<NSURL *> *result))completionHandler {

     NSOpenPanel* openPanel = [NSOpenPanel openPanel];
     openPanel.canChooseFiles = true;
     [openPanel setCanChooseFiles:YES];
     [openPanel setCanChooseDirectories:NO];
     if ( [openPanel runModal] == NSOKButton )
     {
         for( NSURL* URL in [openPanel URLs] )
         {
             NSMutableArray * arr = [NSMutableArray new];
             [arr addObject:URL];
             completionHandler(arr);
         }
     }
     else {
         completionHandler(nil);
     }
}

- (WKWebView*)webView:(WKWebView *)webView
  createWebViewWithConfiguration:(WKWebViewConfiguration *)configuration
  forNavigationAction:(WKNavigationAction *)navigationAction
  windowFeatures:(WKWindowFeatures *)windowFeatures
{
  if (!navigationAction.targetFrame.isMainFrame) {
    NSString* urlStr = navigationAction.request.URL.absoluteString;
    QDesktopServices::openUrl(QUrl(QString::fromNSString(urlStr)));
  }
  return nil;
}

@end

MacWKView* view;

MacWebView::MacWebView(QWidget *parent)
    : WebViewInterface(parent)
{
    m_cocoaView = new QMacCocoaViewContainer(0, this);
    view = [MacWKView alloc];

    NSString* source = @"function captureLog(msg) { window.webkit.messageHandlers.logHandler.postMessage(msg); } window.console.log = captureLog;";

    WKUserScript *script = [[WKUserScript alloc] initWithSource:source injectionTime:WKUserScriptInjectionTimeAtDocumentEnd forMainFrameOnly:YES];
    WKUserContentController *userContentController = [[WKUserContentController alloc] init];
    [userContentController addUserScript:script];
    WKWebViewConfiguration *configuration = [[WKWebViewConfiguration alloc] init];
    configuration.userContentController = userContentController;

    [configuration.userContentController addScriptMessageHandler:view name: @"logHandler"];

    [view initWithFrame:CGRectZero configuration:configuration];
    [view prepareWKWebView];
    view->qtView = this;

    NSURL *url = [NSURL URLWithString:@"https://internal.weapp.io/src/modules/auth/sign-in.html"];
    NSURLRequest *urlReq = [NSURLRequest requestWithURL:url];

    [view loadRequest:urlReq];

    m_cocoaView->setCocoaView(view);
    setupLayout();
}

void MacWebView::setupLayout()
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_cocoaView);
    this->setLayout(mainLayout);
}

void MacWebView::runJavaScript(const QString& js, JsRunType type)
{
    NSString* jsform = js.toNSString();
    [view evaluateJavaScript:jsform completionHandler:^(id Result, NSError * error) {
        if (error == nil) {
            if (Result != nil) {
                switch(type){
                    case JsRunType::TOKEN: {
                        QString token = QString::fromNSString((NSString*)Result);
                        emit tokenReceived(token);
                        if(!token.isEmpty() && m_token.isEmpty()){
                            m_token = token;
                            Logger::instance()->log("MacWebView::runJavaScript(): setIsApp() & setIsRunningApp(false)");
                            this->runJavaScript("setIsApp()");
                            this->runJavaScript("setIsRunningApp(false)");
                        }
                        break;
                    }
                    case JsRunType::ONLINE: {
                        bool isOnline = [Result boolValue];
                        emit connectionChecked(isOnline);
                        break;
                    }
                    case JsRunType::UPLOAD: {
                        QString credentials = QString::fromNSString((NSString*)Result);
                        emit credentialsUploaded(credentials);
                        Logger::instance()->log("Emit Google credentials upload");
                        break;
                    }
                    case JsRunType::CREDENTIALS: {
                        QString credentials = QString::fromNSString((NSString*)Result);
                        emit credentialsTriggered(credentials);
                        break;
                    }
                    case JsRunType::BUCKETNAME: {
                        QString bucketName = QString::fromNSString((NSString*)Result);
                        emit bucketNameTriggered(bucketName);
                        break;
                    }
                    case JsRunType::GETTIME: {
                        QString trackingTime = QString::fromNSString((NSString*)Result);
                        emit trackingTimeObtained(trackingTime);
                        Logger::instance()->log("MacWebView::runJavaScript(): trackingTime == " + trackingTime);
                        break;
                    }
                }
            }
        }
        else {
            NSLog(@"evaluateJavaScript error : %@", error);
        }
    }];
}

void MacWebView::onConsoleMessage(const QString &message)
{
    if(message == "Start tracking" || message == "Stop tracking"){
        emit onButtonStartOrStopPressed(message);
        if(message == "Start tracking")
            requestForCloudCredentialsAndBucketName();
    }
    else if(message == "update_available")
        emit updateButtonPressed();
    else if(message == "google_credentials_upload")
        this->runJavaScript("getUserBucketCredentials()", JsRunType::UPLOAD);
    else if(message == "Google bucket name updated"){
        Logger::instance()->log("MacWebView::onConsoleMessage: Emit Google bucket name updated");
        requestForCloudCredentialsAndBucketName();
    }
    else if(message.startsWith("notification_")){
        Logger::instance()->log("MacWebView::onConsoleMessage: Emit notification received " + message);
        auto list = message.split('_');
        emit notificationTriggered(list[1]);
    }
    else if(message == "autostart on" || message == "autostart off"){
        Logger::instance()->log("MacWebView::onConsoleMessage: autostart == " + message);
        emit onAutoStartSignal(message == "autostart on");
    }

}

void MacWebView::requestForCloudCredentialsAndBucketName()
{
    this->runJavaScript("getUserBucketCredentials()", JsRunType::CREDENTIALS);
    this->runJavaScript("getUserBucketName()", JsRunType::BUCKETNAME);
}

void MacWebView::loadPage(const QString& url)
{
    if(url.isEmpty()){
        m_cocoaView->setCocoaView(view);
        Logger::instance()->log("MacWebView::loadPage(): view is set");
    }
    else
    {
        QUrl qUrl(url);
        NSURL* nsUrl = qUrl.toNSURL();
        NSURLRequest *urlReq = [NSURLRequest requestWithURL:nsUrl];

        [view loadRequest:urlReq];
        m_cocoaView->setCocoaView(view);
    }
}
