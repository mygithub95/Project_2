function Controller()
{
	installer.installationStarted.connect(this, Controller.prototype.onInstallationStarted);
	
}

Controller.prototype.onInstallationStarted = function()
{
	installer.setInstallerBaseBinary(installer.value("TargetDir") + "/maintenancetool.exe");
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
	installer.executeDetached(targetDir + "/WEAPP.exe");	// if any arguments are needed on start, they need to be inserted here
}