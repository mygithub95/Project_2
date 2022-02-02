#---Generation Installer.exe---#

1. Update VERSION  value in  .pro file to the appropriate value using following format [0-9]+(.)[0-9]+(.)[0-9]+(.)[0-9])* 1.1.1.1 .
2. Build Release version of the EXE.
3. Copy build-installer.sh to the folder where is .pro file and run
4. Copy generated WEAPP.app to the MacOS/packages/com.timeteraker.dependencies/data

5. Generate installer.
	
	path to QtInstallerFramework/bin/binarycreator --offline-only -c config/config.xml -p packages -t /path to QtInstallerFramework/bin/installerbase WEAPPInstaller

6․ Generate repository folder
	path to QtInstallerFramework/bin/repogen -p packages repositoryMac
