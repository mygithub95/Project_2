#---Generation Installer.exe---#

1. Update VERSION  value in  .pro file to the appropriate value using following format [0-9]+(.)[0-9]+(.)[0-9]+(.)[0-9])* 1.1.1.1 .
2. Build Release version of the EXE.
3. Copy the EXE to .\packages\com.TimeTracker.executable\data.
3. Update .\packages\com.TimeTracker.executable\meta\package.xml <Version /> to the appropriate value.
4. Copy the DLLs and other project dependencies to .\packages\com.TimeTracker.dependencies\data.
5. open cmd 
6. open this directory 
7. Generate installer.
	.\framework\bin\binarycreator.exe --offline-only -c config\config.xml -p packages WEAPP_1.11.exe

8․ Generate repository folder
	.\framework\bin\repogen.exe -p packages repositoryWindows