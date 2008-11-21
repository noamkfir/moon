//
// System.Windows.Browser.HtmlPage class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;

using Mono;

namespace System.Windows.Browser{

	public static class HtmlPage {
		private static BrowserInformation browser_info = new BrowserInformation ();

		public static BrowserInformation BrowserInformation {
			get {
				return browser_info;
			}
		}
		
		[MonoTODO ("This method should return false when we're not running in a browser.")]
		public static bool IsEnabled {		
		[SecuritySafeCritical ()]
			get {
				return true;
			}
		}
		
		public static HtmlDocument Document {
		[SecuritySafeCritical ()]
			get {
				IntPtr handle = HtmlObject.GetPropertyInternal<IntPtr> (IntPtr.Zero, "document");
				return new HtmlDocument (handle);
			}
		}

		public static void UnregisterCreateableType (string scriptAlias)
		{
			throw new System.NotImplementedException ();
		}
		
		public static void RegisterScriptableObject (string scriptKey, object instance)
		{
			throw new System.NotImplementedException ();
		}
		
		public static void RegisterCreateableType (string scriptAlias, Type type)
		{
			throw new System.NotImplementedException ();
		}

		public static HtmlWindow Window {
			get {
				IntPtr window = HtmlObject.GetPropertyInternal<IntPtr> (IntPtr.Zero, "window");
				//Console.WriteLine ("HtmlPage.Window: {0}", window);
				return new HtmlWindow (window);
			}
		}

		public static HtmlElement Plugin {
			[SecuritySafeCritical]
			get {
				IntPtr obj = NativeMethods.plugin_instance_get_host (Mono.Xaml.XamlLoader.PluginInDomain);
				return new HtmlElement (obj);
			}
		}

		public static bool IsPopupWindowAllowed {
			get { throw new System.NotImplementedException (); }
		}

		public static HtmlWindow PopupWindow (Uri navigateToUri, string target, HtmlPopupWindowOptions options)
		{
			throw new System.NotImplementedException ();
		}
	}
}
