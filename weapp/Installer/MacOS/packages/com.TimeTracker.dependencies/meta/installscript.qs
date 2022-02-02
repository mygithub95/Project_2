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
	component.addElevatedOperation("Execute", "rm", "-rf", "/Applications/WEAPP.app");
	component.addElevatedOperation("Execute", "mkdir", "/Applications/WEAPP.app");
	component.addElevatedOperation("Execute", "mkdir", "/Applications/WEAPP.app/Contents");
	component.addElevatedOperation("Execute", "mkdir", "/Applications/WEAPP.app/Contents/MacOS");
	component.addElevatedOperation("Execute", "mkdir", "/Applications/WEAPP.app/Contents/Resources");
	component.addElevatedOperation("Execute", "cp", installer.value("TargetDir") + "/WEAPP.app/Contents/Resources/main-icon.icns", "/Applications/WEAPP.app/Contents/Resources");
	component.addElevatedOperation("Execute", "cp", installer.value("TargetDir") + "/WEAPP.app/Contents/Info.plist", "/Applications/WEAPP.app/Contents/");
	component.addElevatedOperation("Execute", "ln", "-s", installer.value("TargetDir") + "/WEAPP.app/Contents/MacOS/WEAPP", "/Applications/WEAPP.app/Contents/MacOS/WEAPP", "UNDOEXECUTE", "rm", "-rf", "/Applications/WEAPP.app");

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
