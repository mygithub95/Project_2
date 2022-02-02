function Controller()
{
	installer.installationStarted.connect(this, Controller.prototype.onInstallationStarted);
	
}

Controller.prototype.onInstallationStarted = function()
{
	if (systemInfo.kernelType === "winnt"){
		installer.setInstallerBaseBinary(installer.value("TargetDir") + "/ifw/maintenancetool.exe");
	}
	else if(systemInfo.kernelType === "linux"){
		installer.setInstallerBaseBinary(installer.value("TargetDir") + "/maintenancetool");
	}
	else if(systemInfo.kernelType === "darwin"){
		installer.setInstallerBaseBinary(installer.value("TargetDir") + "/maintenancetool.app/Contents/MacOS/maintenancetool");
	}
}

Controller.prototype.IntroductionPageCallback = function()
{
	gui.clickButton(buttons.UpdaterRadioButton);
	gui.clickButton(buttons.NextButton);
}

Controller.prototype.ComponentSelectionPageCallback = function()
{
	gui.currentPageWidget().selectAll();
	gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function()
{
	gui.clickButton(buttons.CommitButton);
}

Controller.prototype.FinishedPageCallback = function()
{
	gui.clickButton(buttons.FinishButton);
	
	// compute EXE name for variant and restart launcher
	var targetDir = installer.value("TargetDir");	
	if (systemInfo.kernelType === "winnt"){
		var lastIndexOfPathSeparator = targetDir.indexOf("/");
		if(lastIndexOfPathSeparator == -1)
			lastIndexOfPathSeparator = targetDir.indexOf("\\");
		var exeName = targetDir.substring(lastIndexOfPathSeparator + 1) + ".exe";
		installer.executeDetached(targetDir + "/" + exeName);	// if any arguments are needed on start, they need to be inserted here
	}
	else if(systemInfo.kernelType === "linux"){
		installer.executeDetached(targetDir + "/WEAPP_script.sh");
	}
	else if(systemInfo.kernelType === "darwin"){
		installer.executeDetached(targetDir + "/WEAPP.app/Contents/MacOS/WEAPP");
	}
}
