At first build WEAPP.app with

./build-installer.sh

Than sign it with sign.sh file which content is

codesign --deep --verify --verbose -f --options=runtime -i "io.weapp.weapp" --sign "$Developer ID Application: cd onesurgery ltd" WEAPP.app

Than create an installer using this app
Sign installer too with same codesign above

Move installer an empty folder
Create dmg which will includ that folder(Disk utils > File > new Image.....)

Sign dmg installer file 

codesign --deep --force --verify --verbose --options=runtime --sign "Developer ID Application: cd onesurgery ltd" -i "io.weapp.weapp" MacInstallerWEAPP.dmg

Notarize dmg
At first generate password for app Id

xcrun altool --notarize-app --primary-bundle-id "io.weapp.weapp" --username "bettermindss@outlook.com" --password "avqn-xffx-wore-rzrl" --file MacInstallerWEAPP.dmg

Check notarization 

xcrun altool --notarization-info "8d65dbbe-3727-46f5-455-60b37d26b208" -u "bettermindss@outlook.com" -p "avqn-xffx-wore-rzrl"

