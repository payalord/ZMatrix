ZMatrix Source Distro
---------------------

Firstly, I will point out that SourceForge is now hosting the project, and I would like to extend my thanks to them for providing their services.  Please also note the license terms under which the software and source code are distributed below.

License
-------

This is the source code distribution for ZMatrix.  The source code for ZMatrix is provided primarily as a learning tool for all those interested in the techniques employed with the program.  Furthermore, the distribution of the source code is meant to allow for group auditing of the code to ensure it's validity and to provide an avenue for improvement of the code.  It is distributed under the GNU General Public License (GPL).


Copyright (c) 2002, Z. Shaker

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

For further information, I can be reached via email at: zmatrix_background@hotmail.com


Usage
-----

Ok, I'll start by saying right off the bat that chances are that you will have trouble compiling this.  There.  Now that that's out of the way, we can get on with things.  The problem is that the program is divided into modules, some meant to be compiled under Borland c++ Builder (6.0), and some meant to be compiled under Visual C++ (6.0).  Put that on top of the poor source documentation and the fact that I'm likely to have missed some neccessary files, and you've got a problem :0).  The modules that comprimise the program are:

	-zMatrix - The main program.
	-Config - The configuration module.
	-MsgHook - Hook module for catching mouse movements when in screensave mode.
	-zsMatrix - An class representing the matrix streams.
	-ScreenSaver - The source code for creating a .SCR that communicates with the main ZMatrix program.

The zsMatrix class is designed as a COM object because it is passed between the main module (VS 6.0) and the Config module (BCB 6.0).  Also, there is an additional sub-directory called 'Help' which contains the necessary files to compile the compressed HTML help file included with ZMatrix.  The HTML help documents are preprocessed by an excellent perl preprocessor named 'Filepp', before being sent to MS HTML Workshop to be compiled into a .chm file.

zMatrix
-------

This main module takes care of the task of initializing things (creating an instance of the zsMatrix object, creating a main program 'window' and attaching it to the desktop, etc...), providing a UI bridge to the Config module (via the systray icon), updating the zsMatrix object (when a refresh is requested, when the desktop settings have changed), and dealing with program exits and OS shutdowns.

Points of interest in the code are:

-The creation of the main window as a child of the desktop.  See matrix.cpp::FindWindowEx(... "SysListView32" ...).

-The clip region used to prevent overwriting of the desktop icon.  See globals.cpp::UpdateRegions(void) and matrix.cpp::SelectClipRgn(...).

-The capture of the desktop image.  See globals.cpp::UpdateBG(void).


Config
------

This module is a Borland C++ Builder (6.0) DLL which interfaces with the main module via a single function ("LaunchConfigForm").  The method takes as a parameter the current zsMatrix object.  This module is pretty straight forward and simply allows you to modify a number of parameters in the zsMatrix object on the fly.


MsgHook
-------

This module contains hooks for catching mouse move and keyboard messages throughout the system, and detecting changes to the desktop state signalled by windows messages.  It is used to notify the main module when to exit from screen saver mode, and when to update it's snapshot of the desktop state.


zsMatrix
--------

This module represents the core of the matrix streaming effect.  It stores a vector of streams and updates each one.  As has been stated before, this module is designed as a COM object because it must be passed between two modules which are compiled under different compilers.

Points of interest in the code are:

-Stream rendering.  See zsMatrix.cpp::DisplayStreams(...).


ScreenSaver
-----------

This module compiles into a screensaver (.SCR) which is basically an invisible window which opens up and notifies the main ZMatrix executable that it should go into screensave mode.  If the main module is not running, the screensaver launches it, then tells it to go into screensave mode.



Conclusion
----------

So that's about it.  Hopefully it borders on understandable...  Any questions/comments regarding the code should be directed to zmatrix_background@hotmail.com.
