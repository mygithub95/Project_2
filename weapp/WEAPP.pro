QT       += core gui svg network xml

!macx{
QT += webenginewidgets
}

win32 {
    QT += winextras
}

macx {
    QT += macextras
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ActiveEventsTimeProvider.cpp \
    ActivityCalculator.cpp \
    ApiManager.cpp \
    AppInfoProvider.cpp \
    AppTrayIconHandler.cpp \
    AppUpdateController.cpp \
    ControlTracker.cpp \
    CreateVideoFromScreenshots.cpp \
    CustomTimer.cpp \
    DesktopWindow.cpp \
    FFMPEGVideoWriter.cpp \
    FrameWidget.cpp \
    GoogleCloudManager.cpp \
    ImageDownloader.cpp \
    LaunchAppController.cpp \
    Logger.cpp \
    MainWindow.cpp \
    ScreenRecorderThread.cpp \
    TrackingManager.cpp \
    WebViewInterface.cpp \
    main.cpp

HEADERS += \
    ActiveEventsTimeProvider.h \
    ActivityCalculator.h \
    ApiManager.h \
    AppInfoProvider.h \
    AppTrayIconHandler.h \
    AppUpdateController.h \
    ControlTracker.h \
    CreateVideoFromScreenshots.h \
    CustomTimer.h \
    DesktopWindow.h \
    FFMPEGVideoWriter.h \
    FrameWidget.h \
    GoogleCloudManager.h \
    ImageDownloader.h \
    Info.h \
    LaunchAppController.h \
    Logger.h \
    MainWindow.h \
    ScreenRecorderThread.h \
    TrackingManager.h \
    WebViewInterface.h

!macx:{
 SOURCES +=webpage.cpp \
           webview.cpp \
           WebEngineView.cpp \
           SystemTrayHandler.cpp
}

!macx:{
HEADERS += webpage.h \
           webview.h \
           WebEngineView.h \
           SystemTrayHandler.h
}

win32{
SOURCES += WindowsAppInfoProvider.cpp  \
           WindowsBrowserInfoProvider.cpp \
           WinUtilities.cpp \
}

unix: !macx: {
SOURCES += LinuxAppInfoProvider.cpp
}

macx: {
SOURCES += MacAppInfoProvider.mm \
    MacDockHandler.mm \
    MacWebView.mm

}

win32{
HEADERS += WindowsAppInfoProvider.h \
           WindowsBrowserInfoProvider.h \
           WinUtilities.h \
}

unix: !macx: {
HEADERS += LinuxAppInfoProvider.h
}

macx: {
HEADERS += MacAppInfoProvider.h \
    MacWebView.h \
    MacDockHandler.h
}

INCLUDEPATH += $$PWD/Dependencies/ffmpeg/include
INCLUDEPATH += $$PWD/Dependencies/uiohook/include

win32 {
    INCLUDEPATH += $$PWD/Dependencies/google_cloud_storage/include
    INCLUDEPATH += $$PWD/Dependencies/google_cloud_storage/include/grpc

    LIBS += -lNtdll
    LIBS += -lDwmapi

    LIBS += -L$$PWD/Dependencies/ffmpeg/lib
    LIBS += -L$$PWD/Dependencies/uiohook/win/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/win/google/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/win/grpc/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/win/curl/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/win/crc32c/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/win/openssl_win64/lib

    #initial for Cloud Storage
    LIBS += -llibcurl_imp
    LIBS += -llibcrypto_static -lWs2_32 -lAdvapi32 -lUser32
}

unix: !macx:{
    INCLUDEPATH += /usr/include/glib-2.0 /usr/lib/x86_64-linux-gnu/glib-2.0/include
    INCLUDEPATH += $$PWD/Dependencies/google_cloud_storage/include/linux
    INCLUDEPATH += $$PWD/Dependencies/google_cloud_storage/include/linux/grpc

    LIBS += -lglib-2.0  -lX11
    LIBS += -L$$PWD/Dependencies/uiohook/linux/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/linux/google/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/linux/grpc/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/linux/crc32c/lib
}

macx: {
    INCLUDEPATH += $$PWD/Dependencies/google_cloud_storage/include
    INCLUDEPATH += $$PWD/Dependencies/google_cloud_storage/include/grpc

    LIBS += -framework AppKit
    LIBS += -framework Carbon
    LIBS += -framework Webkit
    LIBS += -L/usr/local/lib
    LIBS += -L$$PWD/Dependencies/uiohook/mac/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/mac/google/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/mac/grpc/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/mac/crc32c/lib
    LIBS += -L$$PWD/Dependencies/google_cloud_storage/mac/openssl/lib

    LIBS += -lcurl
    LIBS += -lssl -lcrypto
}

LIBS += -lavdevice
LIBS += -lavformat
LIBS += -lavcodec
LIBS += -lavutil
LIBS += -lswscale
LIBS += -luiohook

LIBS += -lgoogle_cloud_cpp_storage
LIBS += -lgoogle_cloud_cpp_common
LIBS += -labsl_bad_optional_access -labsl_strings_internal -labsl_strings -labsl_throw_delegate -labsl_str_format_internal -labsl_time -labsl_time_zone -labsl_int128 -labsl_raw_logging_internal -labsl_bad_variant_access
LIBS += -lcrc32c

unix: !macx:{
    LIBS += -lcurl
    LIBS += -lssl -lcrypto
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resource.qrc

VERSION = 1.2.2.0
QMAKE_TARGET_PRODUCT = TimeTracker
QMAKE_TARGET_DESCRIPTION = WEAPP

win32 {
    RC_ICONS = main-icon.ico
}

macx: {
    ICON = main-icon.icns
}
