#!/bin/bash 

# -ex

# Put this file to .pro folder
# Install brew and create-dmg too

QTDIR='/usr/local/Qt-5.15.2'
APP='WEAPP.app'
DMG='WEAPP.dmg'
VOLUME_NAME='WEAPP'
MACOSID=''
PUBLISHER_KEY=''
IDENTITY = ''

cleanup() 
{
    make clean
    rm -rf $APP
    rm -f $DMG
}

# Cleanup build data
cleanup

# Building
$QTDIR/bin/qmake WEAPP.pro && make

# Deploing Qt libraries
$QTDIR/bin/macdeployqt $APP -always-overwrite -appstore-compliant 
xattr -c -r $APP

#change library links

FFMPEG_DIR='/usr/local/Cellar/ffmpeg/4.4.1_3/lib'
FRAMEWORKS_DIR='WEAPP.app/Contents/Frameworks'
LOAD_DIR='@executable_path/../Frameworks'
#libavcodec
install_name_tool -change $FFMPEG_DIR/libavutil.56.dylib $LOAD_DIR/libavutil.56.dylib $FRAMEWORKS_DIR/libavcodec.58.dylib
install_name_tool -change $FFMPEG_DIR/libswresample.3.dylib $LOAD_DIR/libswresample.3.dylib $FRAMEWORKS_DIR/libavcodec.58.dylib

#libavdevice
install_name_tool -change $FFMPEG_DIR/libavfilter.7.dylib $LOAD_DIR/libavfilter.7.dylib $FRAMEWORKS_DIR/libavdevice.58.dylib
install_name_tool -change $FFMPEG_DIR/libswscale.5.dylib $LOAD_DIR/libswscale.5.dylib $FRAMEWORKS_DIR/libavdevice.58.dylib
install_name_tool -change $FFMPEG_DIR/libpostproc.55.dylib $LOAD_DIR/libpostproc.55.dylib $FRAMEWORKS_DIR/libavdevice.58.dylib
install_name_tool -change $FFMPEG_DIR/libavformat.58.dylib $LOAD_DIR/libavformat.58.dylib $FRAMEWORKS_DIR/libavdevice.58.dylib
install_name_tool -change $FFMPEG_DIR/libavcodec.58.dylib $LOAD_DIR/libavcodec.58.dylib $FRAMEWORKS_DIR/libavdevice.58.dylib
install_name_tool -change $FFMPEG_DIR/libswresample.3.dylib $LOAD_DIR/libswresample.3.dylib $FRAMEWORKS_DIR/libavdevice.58.dylib
install_name_tool -change $FFMPEG_DIR/libavresample.4.dylib $LOAD_DIR/libavresample.4.dylib $FRAMEWORKS_DIR/libavdevice.58.dylib
install_name_tool -change $FFMPEG_DIR/libavutil.56.dylib $LOAD_DIR/libavutil.56.dylib $FRAMEWORKS_DIR/libavdevice.58.dylib

#libavfilter
install_name_tool -change $FFMPEG_DIR/libswscale.5.dylib $LOAD_DIR/libswscale.5.dylib $FRAMEWORKS_DIR/libavfilter.7.dylib
install_name_tool -change $FFMPEG_DIR/libpostproc.55.dylib $LOAD_DIR/libpostproc.55.dylib $FRAMEWORKS_DIR/libavfilter.7.dylib
install_name_tool -change $FFMPEG_DIR/libavformat.58.dylib $LOAD_DIR/libavformat.58.dylib $FRAMEWORKS_DIR/libavfilter.7.dylib
install_name_tool -change $FFMPEG_DIR/libavcodec.58.dylib $LOAD_DIR/libavcodec.58.dylib $FRAMEWORKS_DIR/libavfilter.7.dylib
install_name_tool -change $FFMPEG_DIR/libswresample.3.dylib $LOAD_DIR/libswresample.3.dylib $FRAMEWORKS_DIR/libavfilter.7.dylib
install_name_tool -change $FFMPEG_DIR/libavresample.4.dylib $LOAD_DIR/libavresample.4.dylib $FRAMEWORKS_DIR/libavfilter.7.dylib
install_name_tool -change $FFMPEG_DIR/libavutil.56.dylib $LOAD_DIR/libavutil.56.dylib $FRAMEWORKS_DIR/libavfilter.7.dylib

#libavresample
install_name_tool -change $FFMPEG_DIR/libavutil.56.dylib $LOAD_DIR/libavutil.56.dylib $FRAMEWORKS_DIR/libavresample.4.dylib

#libavformat
install_name_tool -change $FFMPEG_DIR/libavcodec.58.dylib $LOAD_DIR/libavcodec.58.dylib $FRAMEWORKS_DIR/libavformat.58.dylib
install_name_tool -change $FFMPEG_DIR/libswresample.3.dylib $LOAD_DIR/libswresample.3.dylib $FRAMEWORKS_DIR/libavformat.58.dylib
install_name_tool -change $FFMPEG_DIR/libavutil.56.dylib $LOAD_DIR/libavutil.56.dylib $FRAMEWORKS_DIR/libavformat.58.dylib

#libswscale
install_name_tool -change $FFMPEG_DIR/libavutil.56.dylib $LOAD_DIR/libavutil.56.dylib $FRAMEWORKS_DIR/libswscale.5.dylib

#libswresampl
install_name_tool -change $FFMPEG_DIR/libavutil.56.dylib $LOAD_DIR/libavutil.56.dylib $FRAMEWORKS_DIR/libswresample.3.dylib

#libpostproc
install_name_tool -change $FFMPEG_DIR/libavutil.56.dylib $LOAD_DIR/libavutil.56.dylib $FRAMEWORKS_DIR/libpostproc.55.dylib

#libhogweed
install_name_tool -change /usr/local/Cellar/nettle/3.7.3/lib/libnettle.8.dylib $LOAD_DIR/libnettle.8.dylib $FRAMEWORKS_DIR/libhogweed.6.dylib

#libxcb-shm -shape -xfixes
install_name_tool -change /usr/local/Cellar/libxcb/1.14_1/lib/libxcb.1.dylib $LOAD_DIR/libxcb.1.dylib $FRAMEWORKS_DIR/libxcb-shm.0.dylib
install_name_tool -change /usr/local/Cellar/libxcb/1.14_1/lib/libxcb.1.dylib $LOAD_DIR/libxcb.1.dylib $FRAMEWORKS_DIR/libxcb-shape.0.dylib
install_name_tool -change /usr/local/Cellar/libxcb/1.14_1/lib/libxcb.1.dylib $LOAD_DIR/libxcb.1.dylib $FRAMEWORKS_DIR/libxcb-xfixes.0.dylib

#echo "Signing installer app..."
#codesign --deep --force --verify --verbose --sign "$IDENTITY" $APP

### Simple version
### hdiutil create -volname $VOLUME_NAME -srcfolder $APP -ov -format UDZO $DMG

### Advanced version
#--volicon icon.icns \
#--background background.png \
create-dmg \
--volname $VOLUME_NAME \
--window-pos 0 0 \
--window-size 573 477 \
--icon-size 100 \
--icon $APP 130 280 \
--hide-extension $APP \
--app-drop-link 450 280 \
$DMG $APP

### echo "Signing disk image...(TODO)"
### codesign -i "$MACOSID" -s "$PUBLISHER_KEY" "$OUT_APP_DIR/$DMG_FILENAME" --options runtime
### echo "Checking signature of DMG..."
### codesign  --verbose=4 $DMG

#echo "Notarization...(TODO)"
