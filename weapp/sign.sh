#!/bin/bash

BOUNDLEID='io.weapp.weapp'
IDENTITY='3rd Party Mac Developer Application: cd onesurgery ltd'
INSTALLERIDENTITY='3rd Party Mac Developer Installer: cd onesurgery ltd'
APP='WEAPP.app'
PKG='WEAPP.pkg'
ENTITLEMENTS='weappEntitlements.entitlements'
WE_ENTITLEMENTS='webengine.entitlements'

###copy Info.plist into WEAPP.app
cp Info.plist $APP/Contents/
cp main-icon.icns $APP/Contents/
echo "Signin app..."


codesign --deep --force --verify --verbose --sign "$IDENTITY" --entitlements $ENTITLEMENTS $APP/Contents/MacOS/WEAPP


#codesign --deep --force --verify --verbose --sign "$IDENTITY" --entitlements $WE_ENTITLEMENTS $APP/Contents/Frameworks/QtWebEngineCore.framework/



### remove copied plist file
#rm -f $APP/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/Info.plist  

echo "Generating pkg package..."
productbuild --component $APP /Applications --sign "$INSTALLERIDENTITY" $PKG
