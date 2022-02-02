function Component()
{
    installer.setDefaultPageVisible(QInstaller.Introduction, false);
}

Component.prototype.isDefault = function()
{
    // select the component by default
    return true;
}

Component.prototype.createOperations = function()
{
    try {
        // call the base create operations function
        component.createOperations();
		if (systemInfo.productType === "windows") {
			component.addOperation("CreateShortcut", "@TargetDir@/WEAPP.exe", "@DesktopDir@/WEAPP.lnk");
		}
		if (systemInfo.kernelType === "linux") {   
			component.addElevatedOperation("Execute", "/bin/bash", installer.value("TargetDir") + "/setTargetPath.sh", installer.value("TargetDir"))
			component.addElevatedOperation("Execute", "mv", installer.value("TargetDir") + "/WEAPP.desktop",  "/usr/share/applications/", "UNDOEXECUTE", "rm", "-f", "/usr/share/applications/WEAPP.desktop");
		}
    } catch (e) {
        console.log(e);
    }
}

Component.prototype.loaded = function ()
{

}

Component.prototype.dynamicPageEntered = function ()
{
}
