//
// Unit tests for TextBlock
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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

using System;
using System.Windows;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class TextBlockTest {

		[TestMethod]
		public void NullifyFontFamily ()
		{
			TextBlock tb = new TextBlock ();
			tb.FontFamily = null;
		}

		[TestMethod]
		[MoonlightBug]
		public void MeasureTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello";
			
			b.Child = tb;
			
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (new Size (28,16), tb.DesiredSize, "tb.DesiredSize 0");
		}

		[TestMethod]
		[MoonlightBug]
		public void MeasureNewlineTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you\nforget Who I am";
			
			b.Child = tb;
			
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (new Size (107,32), tb.DesiredSize, "tb.DesiredSize 0");
		}

		[TestMethod]
		[MoonlightBug]
		public void MeasureTooLongLineTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you forget Who I am";
			
			b.Child = tb;
			b.Width = 44;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (new Size (44,16), tb.DesiredSize, "tb.DesiredSize 0");
		}

		[TestMethod]
		[MoonlightBug]
		public void MeasureTooLongLineWrapTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you forget Who I am";
			tb.TextWrapping = TextWrapping.Wrap;

			b.Child = tb;
			b.Width = 44;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (new Size (33,112), tb.DesiredSize, "tb.DesiredSize 0");
		}

		[TestMethod]
		[MoonlightBug]
		public void ArrangeTooLongLineTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.Text = "Hello and don't you forget Who I am";
			
			b.Child = tb;
			b.Width = 44;

			Assert.IsTrue (tb.ActualWidth < 202.4 && tb.ActualWidth > 202.3, "tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (16, tb.ActualHeight, "tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.AreEqual (new Size (44,16), tb.DesiredSize, "tb.DesiredSize 0");
			Assert.AreEqual (new Size (44,16), b.DesiredSize, "1b.DesiredSize 0");
			Assert.IsTrue (tb.ActualWidth < 202.4 && tb.ActualWidth > 202.3, "tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (16, tb.ActualHeight, "tb.ActualHeight");

			b.Arrange (new Rect (0,0, b.DesiredSize.Width, b.DesiredSize.Height));

			Assert.AreEqual (new Size (44,16), tb.DesiredSize, "tb.DesiredSize 1");
			Assert.AreEqual (new Size (44,16), b.DesiredSize, "b.DesiredSize 1");
			Assert.IsTrue (tb.ActualWidth > 202.3 && tb.ActualWidth < 202.4,"tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (16, tb.ActualHeight, "tb.ActualHeight");
			Assert.AreEqual (new Size (44,16), new Size (b.ActualWidth, b.ActualHeight), "b.Actual*");
		}

		[TestMethod]
		[MoonlightBug]
		public void ArrangeTooLongLineWrapMeasureTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			tb.Text = "Hello and don't you forget Who I am";
			// notice this is on the border not the textblock
			b.Width = 44;

			Assert.IsTrue (tb.ActualWidth < 202.4 && tb.ActualWidth > 202.3, "tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (16, tb.ActualHeight, "tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "tb.ActualWidth1 is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (560, tb.ActualHeight, "tb.ActualHeight1");
		}

		[TestMethod]
		[MoonlightBug]
		public void ArrangeTooLongLocal_LineWrapMeasureTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			tb.Text = "Hello and don't you forget Who I am";
			// notice this is on the textblock not the border
			tb.Width = 44;

			Assert.IsTrue (tb.ActualWidth < 34 && tb.ActualWidth > 33, "tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (112, tb.ActualHeight, "tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "tb.ActualWidth1 is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (560, tb.ActualHeight, "tb.ActualHeight1");
		}

		[TestMethod]
		[MoonlightBug]
		public void ArrangeTooLongLineWrapMeasureReverseTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			b.Width = 44;
			tb.Text = "Hello and don't you forget Who I am";

			Assert.IsTrue (tb.ActualWidth < 202.4 && tb.ActualWidth > 202.3, "tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (16, tb.ActualHeight, "tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "tb.ActualWidth1 is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (560, tb.ActualHeight, "tb.ActualHeight1");
		}

		[TestMethod]
		[MoonlightBug]
		public void ArrangeTooLongLocal_LineWrapMeasureReverseTest ()
		{
			Border b = new Border ();
			TextBlock tb = new TextBlock ();
			tb.TextWrapping = TextWrapping.Wrap;
			
			b.Child = tb;
			tb.Width = 44;
			tb.Text = "Hello and don't you forget Who I am";

			Assert.AreEqual (11, tb.FontSize);
			Assert.IsTrue (tb.ActualWidth < 34 && tb.ActualWidth > 33, "tb.ActualWidth is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (112, tb.ActualHeight, "tb.ActualHeight");

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.AreEqual (11, tb.FontSize);
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "tb.ActualWidth1 is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (560, tb.ActualHeight, "tb.ActualHeight1");

			tb.Text = "Hello and don't you forget Who I am.";

			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "tb.ActualWidth2 is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (576, tb.ActualHeight, "tb.ActualHeight2");

			tb.Width = 70;
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));

			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "tb.ActualWidth3 is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (576, tb.ActualHeight, "tb.ActualHeight3");

			b.InvalidateMeasure ();
			tb.InvalidateMeasure ();

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			
			Assert.IsTrue (tb.ActualWidth < 10.9 && tb.ActualWidth > 10.8, "tb.ActualWidth4 is " + tb.ActualWidth.ToString ());
			Assert.AreEqual (576, tb.ActualHeight, "tb.ActualHeight4");
		}

		[TestMethod]
		[MoonlightBug]
		public void ComputeActualWidth ()
		{
			var c = new TextBlock ();
			
			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c.DesiredSize 0");
			Assert.AreEqual (new Size (0,0), new Size (c.ActualWidth,c.ActualHeight), "c.Actual 0");
			
			c.Text = "Hello";

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired");
			Assert.IsTrue (c.ActualWidth < 27.6 && c.ActualWidth > 27.5, "c.ActualWidth is " + c.ActualWidth.ToString ());
			Assert.AreEqual (16, c.ActualHeight, "c.ActualHeight");

			c.MaxWidth = 25;
			c.Width = 50;
			c.MinHeight = 33;

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired1");
			Assert.IsTrue (c.ActualWidth < 27.6 && c.ActualWidth > 27.5, "c.ActualWidth1 is " + c.ActualWidth.ToString ());
			Assert.AreEqual (16, c.ActualHeight, "c.ActualHeight1");

			c.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (0,0), c.DesiredSize, "c desired2");
			Assert.IsTrue (c.ActualWidth < 27.6 && c.ActualWidth > 27.5, "c.ActualWidth2 is " + c.ActualWidth.ToString ());
			Assert.AreEqual (16, c.ActualHeight, "c.ActualHeight2");
		}
	}
}
