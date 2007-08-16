/*
 * moon-plugin.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "plugin.h"
#include "plugin-class.h"
#include "moon-mono.h"
#include "downloader.h"

/* gleaned from svn log of the moon module, as well as olive/class/{agclr,agmono,System.Silverlight} */
static const gchar *moonlight_authors[] = {
	"Andreia Gaita <avidigal@novell.com>",
	"Atsushi Enomoto <atsushi@ximian.com>",
	"Chris Toshok <toshok@ximian.com>",
	"Dick Porter <dick@ximian.com>",
	"Everaldo Canuto <ecanuto@novell.com>",
	"Jackson Harper <jackson@ximian.com>",
	"Jeffrey Stedfast <fejj@novell.com>",
	"Larry Ewing <lewing@novell.com>",
	"Marek Habersack <mhabersack@novell.com>",
	"Miguel de Icaza <miguel@novell.com>",
	"Rodrigo Kumpera <rkumpera@novell.com>",
	"Rolf Bjarne Kvinge <RKvinge@novell.com>",
	"Sebastien Pouliot <sebastien@ximian.com>",
	NULL
};

void
plugin_menu_about (PluginInstance *plugin)
{
	GtkAboutDialog *about = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());

	gtk_about_dialog_set_name (about, PLUGIN_OURNAME);
	gtk_about_dialog_set_version (about, PLUGIN_OURVERSION);

	gtk_about_dialog_set_copyright (about, "Copyright 2007 Novell, Inc. (http://www.novell.com/)");
	gtk_about_dialog_set_website (about, "http://mono-project.com/Moonlight/");
	gtk_about_dialog_set_website_label (about, "Project Website");

	gtk_about_dialog_set_authors (about, moonlight_authors);

	gtk_dialog_run (GTK_DIALOG (about));
}

void
plugin_show_menu (PluginInstance *plugin)
{
	GtkWidget *menu;
	GtkWidget *menu_item;
	char *name;

	menu = gtk_menu_new();
	
	name = g_strdup_printf ("%s %s", PLUGIN_OURNAME, PLUGIN_OURVERSION);
	menu_item = gtk_menu_item_new_with_label (name);
	g_free (name);
	
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	g_signal_connect_swapped (G_OBJECT(menu_item), "activate", G_CALLBACK (plugin_menu_about), plugin);

	gtk_widget_show_all (menu);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
}

gboolean
plugin_event_callback (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gboolean handled = 0;

	PluginInstance *plugin = (PluginInstance *) user_data;
	GdkEventButton *event_button;

	switch (event->type) {

		case GDK_BUTTON_PRESS:
			event_button = (GdkEventButton *) event;
			if (event_button->button == 3) {
				plugin_show_menu (plugin);
			}
			handled = 1;
			break;

		default:
			break;
	}

	return handled;
}

/*** PluginInstance:: *********************************************************/

GSList *plugin_instances = NULL;

PluginInstance::PluginInstance (NPP instance, uint16 mode)
{
	this->mode = mode;
	this->instance = instance;
	this->window = NULL;

	this->rootobject = NULL;

	this->container = NULL;
	this->surface = NULL;

	// Property fields
	this->initParams = false;
	this->isLoaded = false;
	this->source = NULL;
	this->onLoad = NULL;
	this->onError = NULL;
	this->background = NULL;

	this->windowless = false;
	
	this->vm_missing_file = NULL;
	this->mono_loader_object = NULL;

	this->timers = NULL;

	plugin_instances = g_slist_append (plugin_instances, this->instance);

	/* back pointer to us */
	instance->pdata = this;
}

PluginInstance::~PluginInstance ()
{
	// Kill timers
	GSList *p;
	for (p = timers; p != NULL; p = p->next){
		uint32_t source_id = (uint32_t) p->data;

		g_source_remove (source_id);
	}
	g_slist_free (p);
	
	// Remove us from the list.
	plugin_instances = g_slist_remove (plugin_instances, this->instance);

	// 
	// The code below was an attempt at fixing this, but we are still getting spurious errors
	// we might have another source of problems
	//
	fprintf (stderr, "Destroying the plugin: %p\n", surface);
	if (surface != NULL){
		//gdk_error_trap_push ();
		delete surface;
		//gdk_display_sync (this->display);
		//gdk_error_trap_pop ();
	}

	if (rootobject)
		NPN_ReleaseObject ((NPObject*)rootobject);

	if (background)
		g_free (background);
}

void 
PluginInstance::Initialize (int argc, char* const argn[], char* const argv[])
{
	for (int i = 0; i < argc; i++) {
		if (argn[i] == NULL)
			continue;

		// initParams.
		if (!strcasecmp (argn[i], "initParams")) {
			this->initParams = argv[i];
		}

		// onLoad.
		if (!strcasecmp (argn[i], "onLoad")) {
			this->onLoad = argv[i];
		}

		// onError.
		if (!strcasecmp (argn[i], "onError")) {
			this->onError = argv[i];
		}

		// Source url handle.
		if (!strcasecmp (argn[i], "src") || !strcasecmp (argn[i], "source")) {
			this->source = argv[i];
		}

		if (!strcasecmp (argn[i], "background")) {
			this->background = g_strdup (argv[i]);
		}
	}
}

void 
PluginInstance::Finalize ()
{
}

NPError 
PluginInstance::GetValue (NPPVariable variable, void *result)
{
	NPError err = NPERR_NO_ERROR;

	switch (variable) {
		case NPPVpluginNeedsXEmbed:
			*((PRBool *)result) = PR_TRUE;
			break;

		case NPPVpluginScriptableNPObject:
			*((NPObject**) result) = getRootObject ();
			break;
		default:
			err = NPERR_INVALID_PARAM;
	}

	return err;
}

NPError
PluginInstance::SetValue (NPNVariable variable, void *value)
{
	return NPERR_NO_ERROR;
}

NPError 
PluginInstance::SetWindow (NPWindow* window)
{
	if (window == this->window)
		return NPERR_NO_ERROR;

	NPN_GetValue(this->instance, NPNVSupportsXEmbedBool, &this->xembed_supported);
	if (!this->xembed_supported)
	{
		DEBUGMSG ("*** XEmbed not supported");
		return NPERR_GENERIC_ERROR;
	}

	this->window = window;
	this->CreateWindow ();

	return NPERR_NO_ERROR;
}

void 
PluginInstance::CreateWindow ()
{
	DEBUGMSG ("*** creating window (%d,%d,%d,%d)", window->x, window->y, window->width, window->height);

	//  GtkPlug container and surface inside
	this->container = gtk_plug_new (reinterpret_cast <GdkNativeWindow> (window->window));

	// Connect signals to container
	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (this->container), GTK_CAN_FOCUS);

	gtk_widget_add_events (
		this->container,
		GDK_BUTTON_PRESS_MASK | 
		GDK_BUTTON_RELEASE_MASK |
		GDK_KEY_PRESS_MASK | 
		GDK_KEY_RELEASE_MASK | 
		GDK_POINTER_MOTION_MASK |
		GDK_SCROLL_MASK |
		GDK_EXPOSURE_MASK |
		GDK_VISIBILITY_NOTIFY_MASK |
		GDK_ENTER_NOTIFY_MASK |
		GDK_LEAVE_NOTIFY_MASK |
		GDK_FOCUS_CHANGE_MASK
	);

	g_signal_connect (G_OBJECT(this->container), "event", G_CALLBACK (plugin_event_callback), this);

	this->surface = new Surface (window->width, window->height);

	if (background) {
		Color *c = color_from_str (background);
		surface->SetBackgroundColor (c);
		delete c;
	}

	gtk_container_add (GTK_CONTAINER (container), this->surface->GetDrawingArea());
	display = gdk_drawable_get_display (this->surface->GetDrawingArea()->window);
	gtk_widget_show_all (this->container);
	this->UpdateSource ();
}

void 
PluginInstance::UpdateSource ()
{
	if (!this->source)
		return;

	char *pos = strchr (this->source, '#');
	if (pos) {
		if (strlen(&pos[1]) > 0);
			this->UpdateSourceByReference (&pos[1]);
	} else {
		StreamNotify *notify = new StreamNotify (StreamNotify::SOURCE, this->source);
		NPN_GetURLNotify (this->instance, this->source, NULL, notify);
	}
}

void 
PluginInstance::UpdateSourceByReference (const char *value)
{
	NPObject *object = NULL;
	NPString reference;
	NPVariant result;

	if (NPERR_NO_ERROR != NPN_GetValue(this->instance, NPNVWindowNPObject, &object)) {
		DEBUGMSG ("*** Failed to get window object");
		return;
	}

	char jscript[strlen (value) + strlen (".text") + 1];

	g_strlcpy (jscript, value, sizeof(jscript));
	g_strlcat (jscript, ".text", sizeof(jscript));

	reference.utf8characters = jscript;
	reference.utf8length = strlen (jscript);

	if (NPN_Evaluate(this->instance, object, &reference, &result)) {
		if (NPVARIANT_IS_STRING (result)) {
			mono_loader_object = vm_xaml_str_loader_new (this, this->surface, NPVARIANT_TO_STRING (result).utf8characters);
			TryLoad ();
		}

		NPN_ReleaseVariantValue (&result);
	}

	NPN_ReleaseObject (object);
}

bool
PluginInstance::JsRunOnload ()
{
	bool retval = false;
	NPObject *object = NULL;
	NPVariant result;
	const char *expression = onLoad;

	if (NPERR_NO_ERROR != NPN_GetValue(instance, NPNVWindowNPObject, &object)) {
		DEBUGMSG ("*** Failed to get window object");
		return false;
	}

	if (!strncmp (expression, "javascript:", strlen ("javascript:")))
		expression += strlen ("javascript:");

	NPVariant args[1];

	DependencyObject *toplevel = surface->GetToplevel ();
	DEBUGMSG ("In JsRunOnload, toplevel = %p", toplevel);

	MoonlightDependencyObjectObject *depobj = DependencyObjectCreateWrapper (instance, toplevel);
	OBJECT_TO_NPVARIANT ((NPObject*)depobj, args[0]);

	if (NPN_Invoke (instance, object, NPID (expression),
			args, 1, &result)) {

		DEBUGMSG ("NPN_Invoke succeeded");
		NPN_ReleaseVariantValue (&result);

		return true;
	}
	else {
		DEBUGMSG ("NPN_Invoke failed");
	}

	NPN_ReleaseObject (object);
	
	return retval;
}

NPError
PluginInstance::NewStream (NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
  //	DEBUGMSG ("NewStream (%s) %s", this->source, stream->url);

	if (IS_NOTIFY_SOURCE (stream->notifyData)) {
		*stype = NP_ASFILEONLY;
		return NPERR_NO_ERROR;
	} 

	if (IS_NOTIFY_DOWNLOADER (stream->notifyData)) {
		*stype = NP_ASFILE;
		return NPERR_NO_ERROR;
	} 

	if (IS_NOTIFY_REQUEST (stream->notifyData)) {
		*stype = NP_ASFILEONLY;
		return NPERR_NO_ERROR;
	}

	*stype = NP_NORMAL;

	return NPERR_NO_ERROR;
}

NPError
PluginInstance::DestroyStream (NPStream* stream, NPError reason)
{
	return NPERR_NO_ERROR;
}

//
// Tries to load the XAML file, the parsing might fail because a
// required dependency is not available, so we need to queue the
// request to fetch the data.
//
void
PluginInstance::TryLoad ()
{
	int error = 0;

	vm_missing_file = vm_loader_try (mono_loader_object, &error);

	if (vm_missing_file != NULL){
		StreamNotify *notify = new StreamNotify (StreamNotify::REQUEST, vm_missing_file);
		NPN_GetURLNotify (instance, vm_missing_file, NULL, notify);
		return;
	}

	//
	// missing file was NULL, if error is set, display some message
	//
	if (!this->isLoaded && surface->GetToplevel ()) {
		this->isLoaded = true;
		if (this->onLoad)
			JsRunOnload ();
	}
}

void
PluginInstance::StreamAsFile (NPStream* stream, const char* fname)
{
  //	DEBUGMSG ("StreamAsFile: %s", fname);

	if (IS_NOTIFY_SOURCE (stream->notifyData)) {
	  //		DEBUGMSG ("LoadFromXaml: %s", fname);
		mono_loader_object = vm_xaml_file_loader_new (this, this->surface, fname);
		TryLoad ();
	}

	if (IS_NOTIFY_DOWNLOADER (stream->notifyData)){
		Downloader * dl = (Downloader *) ((StreamNotify *)stream->notifyData)->pdata;

		downloader_notify_finished (dl, fname);
	}
	
	if (IS_NOTIFY_REQUEST (stream->notifyData)) {
		vm_insert_mapping (mono_loader_object, vm_missing_file, fname);
		vm_insert_mapping (mono_loader_object, stream->url, fname);
		g_free (vm_missing_file);
		vm_missing_file = NULL;

		// retry to load
		TryLoad ();
	}
}

int32
PluginInstance::WriteReady (NPStream* stream)
{
	//DEBUGMSG ("WriteReady (%d)", stream->end);

	StreamNotify *notify = STREAM_NOTIFY (stream->notifyData);

	if (notify && notify->pdata && IS_NOTIFY_DOWNLOADER (notify)) {
		Downloader * dl = (Downloader *) notify->pdata;
		downloader_notify_size (dl, stream->end);
		return MAX_STREAM_SIZE;
	}
	
	NPN_DestroyStream (instance, stream, NPRES_DONE);

	return -1L;
}

int32
PluginInstance::Write (NPStream* stream, int32 offset, int32 len, void* buffer)
{
	//DEBUGMSG ("Write size: %d offset: %d len: %d", stream->end, offset, len);

	StreamNotify *notify = STREAM_NOTIFY (stream->notifyData);

	if (notify && notify->pdata && IS_NOTIFY_DOWNLOADER (notify)) {
		Downloader * dl = (Downloader *) notify->pdata;
		downloader_write (dl, (guchar*) buffer, 0, len);
	}

	return len;
}

// Defined in plugin/downloader.cpp
void downloader_notify (const char* url, NPReason reason, StreamNotify *notify);

void
PluginInstance::UrlNotify (const char* url, NPReason reason, void* notifyData)
{
	StreamNotify *notify = (StreamNotify *) notifyData;

	switch (notify->type) {
	case StreamNotify::NONE:
	case StreamNotify::SOURCE:
	case StreamNotify::REQUEST:
		// for now I don't think any special actions are needed here
		break;
	case StreamNotify::DOWNLOADER:
		downloader_notify (url, reason, notify);
		break;
	}
}

void
PluginInstance::Print (NPPrint* platformPrint)
{
	// nothing to do.
}

int16
PluginInstance::EventHandle (void* event)
{
	return 0;
}

/*** Getters and Setters ******************************************************/

void
PluginInstance::setSource (const char *value)
{
	if (!value || (this->source && !strcasecmp (this->source, value)))
		return;

	this->source = (char *) NPN_MemAlloc (strlen (value) + 1);
	strcpy (this->source, value);
	this->UpdateSource ();
}

char *
PluginInstance::getBackground ()
{
	return background;
}

void
PluginInstance::setBackground (const char *value)
{
	if (background)
		g_free (background);
	background = g_strdup (value);
	if (surface) {
		Color *c = color_from_str (background);
		surface->SetBackgroundColor (c);
		delete c;
	}
}

bool
PluginInstance::getEnableFramerateCounter ()
{
	return false;
}

bool
PluginInstance::getEnableRedrawRegions ()
{
	return false;
}

void
PluginInstance::setEnableRedrawRegions (bool value)
{
	// not implemented yet.
}

bool
PluginInstance::getEnableHtmlAccess ()
{
	return true;
}

bool
PluginInstance::getWindowless ()
{
	return this->windowless;
}

int32
PluginInstance::getActualHeight ()
{
	return window->height;
}

int32
PluginInstance::getActualWidth ()
{
	return window->width;
}

void
PluginInstance::getBrowserInformation (char **name, char **version,
				       char **platform, char **userAgent,
				       bool *cookieEnabled)
{
	*userAgent = (char*)NPN_UserAgent (instance);
	DEBUG_WARN_NOTIMPLEMENTED ("pluginInstance.getBrowserInformation");

	*name = "Foo!";
	*version = "Foo!";
	*platform = "Foo!";
	*cookieEnabled = true;
}

MoonlightScriptControlObject *
PluginInstance::getRootObject ()
{
	if (rootobject == NULL)
		rootobject = NPN_CreateObject (instance, MoonlightScriptControlClass);

	NPN_RetainObject (rootobject);
	return (MoonlightScriptControlObject*)rootobject;
}

NPP
PluginInstance::getInstance ()
{
	return instance;
}

int32
plugin_instance_get_actual_width (PluginInstance *instance)
{
	return instance->getActualWidth ();
}

int32
plugin_instance_get_actual_height (PluginInstance *instance)
{
	return instance->getActualHeight ();
}

char*
plugin_instance_get_init_params  (PluginInstance *instance)
{
	return instance->getInitParams();
}

uint32_t
plugin_html_timer_timeout_add (PluginInstance *instance, int32_t interval, GSourceFunc callback, gpointer data)
{
	uint32_t id;

#if GLIB_CHECK_VERSION(2,14,0)
	if (interval > 1000 && ((interval % 1000) == 0))
		id = g_timeout_add_seconds (interval / 1000, callback, data);
	else
#endif
		id = g_timeout_add (interval, callback, data);

	instance->timers = g_slist_append (instance->timers, GINT_TO_POINTER ((int)id));

	return id;
}

void
plugin_html_timer_timeout_stop (PluginInstance *instance, uint32_t source_id)
{
	g_source_remove (source_id);
	instance->timers = g_slist_remove (instance->timers, GINT_TO_POINTER (source_id));
}


void
plugin_instance_get_browser_information (PluginInstance *instance,
					 char **name, char **version,
					 char **platform, char **userAgent,
					 bool *cookieEnabled)
{
	instance->getBrowserInformation (name, version, platform, userAgent, cookieEnabled);
}
