#!/bin/sh
LD_LIBRARY_PATH=path/lib/:${LD_LIBRARY_PATH}
LD_LIBRARY_PATH=path/nss/:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH
QT_PLUGIN_PATH=path/lib/qt5/plugins:${QT_PLUGIN_PATH}
export QT_PLUGIN_PATH
path/WEAPP