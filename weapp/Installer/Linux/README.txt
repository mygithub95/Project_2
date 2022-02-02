#---Generation Installer.exe---#

1. Update VERSION  value in  .pro file to the appropriate value using following format [0-9]+(.)[0-9]+(.)[0-9]+(.)[0-9])* 1.1.1.1 .
2. Build Release version of the EXE.
3. Copy the executable file to .\packages\com.TimeTracker.executable\data.
   Copy main-icon.png and main-icon.ico to .\packages\com.TimeTracker.executable\data.
3. Update .\packages\com.TimeTracker.dependencies\meta\package.xml <Version /> to the appropriate value.
4. Update .\packages\com.TimeTracker.executable\meta\package.xml <Version /> to the appropriate value.
5. Copy the libs and other project dependencies to .\packages\com.TimeTracker.dependencies\data\lib.
6. open terminal
7. open this directory 
8. Generate installer.
	For linux.
	path to QtInstallerFramework/bin/binarycreator --offline-only -c config/config.xml -p packages -t /path to QtInstallerFramework/bin/installerbase WEAPPInstaller

9․ Generate repository folder
	path to QtInstallerFramework/bin/repogen -p packages repositoryLinux
