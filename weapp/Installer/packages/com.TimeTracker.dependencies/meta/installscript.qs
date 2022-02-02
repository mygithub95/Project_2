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
