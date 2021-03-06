const nsISupports           = Components.interfaces.nsISupports;
const nsICategoryManager    = Components.interfaces.nsICategoryManager;
const nsIComponentRegistrar = Components.interfaces.nsIComponentRegistrar;
const nsICommandLine        = Components.interfaces.nsICommandLine;
const nsICommandLineHandler = Components.interfaces.nsICommandLineHandler;
const nsIFactory            = Components.interfaces.nsIFactory;
const nsIModule             = Components.interfaces.nsIModule;
const nsIWindowWatcher      = Components.interfaces.nsIWindowWatcher;
const nsIConsoleService     = Components.interfaces.nsIConsoleService;
const nsIMutableArray       = Components.interfaces.nsIMutableArray;
const nsIWritableVariant    = Components.interfaces.nsIWritableVariant;

// the chrome URI of our extension
const CHROME_URI = "chrome://moonlight/content/";

// the contract id, CID, and category
const clh_contractID = "@mozilla.org/commandlinehandler/general-startup;1?type=moonlight";

// use uuidgen to generate a unique ID
const clh_CID = Components.ID("{f066e3d7-71e0-44eb-8e41-a445b0753125}");

// category names are sorted alphabetically. Typical command-line handlers use a
// category that begins with the letter "m".
const clh_category = "m-moonlight";

/**
 * Utility functions
 */

function myDump(aMessage) {
  var consoleService = Components.classes["@mozilla.org/consoleservice;1"]
                                 .getService(nsIConsoleService);
  consoleService.logStringMessage("Moonlight: " + aMessage);
}

/**
 * Opens a chrome window.
 * @param aChromeURISpec a string specifying the URI of the window to open.
 * @param aArgument an argument to pass to the window (may be null)
 */
function openWindow(aChromeURISpec)
{
  var array = Components.classes["@mozilla.org/array;1"]
                          .createInstance(nsIMutableArray);
  for (var i=1; i<arguments.length; i++)
  {
    myDump ("arguments[" + i + "] = " + arguments[i]);

    var variant = Components.classes["@mozilla.org/variant;1"]
		    .createInstance(nsIWritableVariant);
    variant.setFromVariant(arguments[i]);
    array.appendElement(variant, false);
  }

  var ww = Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
	     .getService(nsIWindowWatcher);
  ww.openWindow(null, aChromeURISpec, "_blank",
		"chrome,resizable,dialog=no",
		array);
}

/**
 * The XPCOM component that implements nsICommandLineHandler.
 * It also implements nsIFactory to serve as its own singleton factory.
 */
const moonlightHandler = {
  /* nsISupports */
  QueryInterface : function clh_QI(iid)
  {
    if (iid.equals(nsICommandLineHandler) ||
        iid.equals(nsIFactory) ||
        iid.equals(nsISupports))
      return this;

    throw Components.results.NS_ERROR_NO_INTERFACE;
  },

  /* nsICommandLineHandler */

  handle : function clh_handle(cmdLine)
  {
    try {
      // command line flag that takes an argument
      var uristr = cmdLine.handleFlagWithParam("moonapp", false);
      var width = cmdLine.handleFlagWithParam("moonwidth", false);
      var height = cmdLine.handleFlagWithParam("moonheight", false);
      var title = cmdLine.handleFlagWithParam("moontitle", false);

      // only open a window if they supply an app for us to load.
      if (!uristr)
	return;

      myDump ("opening uri " + uristr + ", width = " + width + ", height = " + height);

      // convert uristr to an nsIURI
      var uri = cmdLine.resolveURI(uristr);

      width = parseInt (width);
      if (isNaN (width)) width = 500;

      height = parseInt (height);
      if (isNaN (height)) height = 500;

      if (!title)
	title = "Moonlight Out of Browser Application";
      
      openWindow(CHROME_URI, uri, title, width, height);
      cmdLine.preventDefault = true;
    }
    catch (e) {
      Components.utils.reportError("incorrect parameter passed to -moonapp on the command line.");
      Components.utils.reportError(e);
    }
  },

  // CHANGEME: change the help info as appropriate, but
  // follow the guidelines in nsICommandLineHandler.idl
  // specifically, flag descriptions should start at
  // character 24, and lines should be wrapped at
  // 72 characters with embedded newlines,
  // and finally, the string should end with a newline
  helpInfo : "  -moonapp <uri>       Open the specified Silverlight application\n" +
             "                       in Out-of-Browser mode.\n" +
	     "  -moonwidth <int>     Specifies the width of the Out-of-Browser window.\n" +
	     "  -moonheight <int>    Specifies the height of the Out-of-Browser window.\n" +
	     "  -moontitle <str>     Specifies the window title of the Out-of-Browser window.\n",

  /* nsIFactory */

  createInstance : function clh_CI(outer, iid)
  {
    if (outer != null)
      throw Components.results.NS_ERROR_NO_AGGREGATION;

    return this.QueryInterface(iid);
  },

  lockFactory : function clh_lock(lock)
  {
    /* no-op */
  }
};

/**
 * The XPCOM glue that implements nsIModule
 */
const moonlightHandlerModule = {
  /* nsISupports */
  QueryInterface : function mod_QI(iid)
  {
    if (iid.equals(nsIModule) ||
        iid.equals(nsISupports))
      return this;

    throw Components.results.NS_ERROR_NO_INTERFACE;
  },

  /* nsIModule */
  getClassObject : function mod_gch(compMgr, cid, iid)
  {
    if (cid.equals(clh_CID))
      return moonlightHandler.QueryInterface(iid);

    throw Components.results.NS_ERROR_NOT_REGISTERED;
  },

  registerSelf : function mod_regself(compMgr, fileSpec, location, type)
  {
    compMgr.QueryInterface(nsIComponentRegistrar);

    compMgr.registerFactoryLocation(clh_CID,
                                    "moonlightHandler",
                                    clh_contractID,
                                    fileSpec,
                                    location,
                                    type);

    var catMan = Components.classes["@mozilla.org/categorymanager;1"].
      getService(nsICategoryManager);
    catMan.addCategoryEntry("command-line-handler",
                            clh_category,
                            clh_contractID, true, true);
  },

  unregisterSelf : function mod_unreg(compMgr, location, type)
  {
    compMgr.QueryInterface(nsIComponentRegistrar);
    compMgr.unregisterFactoryLocation(clh_CID, location);

    var catMan = Components.classes["@mozilla.org/categorymanager;1"].
      getService(nsICategoryManager);
    catMan.deleteCategoryEntry("command-line-handler", clh_category);
  },

  canUnload : function (compMgr)
  {
    return true;
  }
};

/* The NSGetModule function is the magic entry point that XPCOM uses to find what XPCOM objects
 * this component provides
 */
function NSGetModule(comMgr, fileSpec)
{
  return moonlightHandlerModule;
}
