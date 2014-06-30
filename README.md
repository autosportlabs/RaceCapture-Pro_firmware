RaceCapture App
===============

Next Gen version of the RaceCapture App.

Dependencies:
* Python 2.7.x
* Kivy 1.8.x
* Pyserial 2.6.x
* Pygame
* Included in package right now but ultimately shouldn't be:
  * Graph (via kivy-garden via python-pip)
  * NavigationDrawer (via kivy-garden)

## dev installation (OS X)

1. download [Kivy](http://kivy.org/#download) (current is 1.8.0)
1. follow Kivy install instructions
- Chris Rae: I found step 3 a bit easier installing pip (`sudo easy_install pip`) and then virtualenv (`sudo pip install virtualenv`). But I don't know about Macs so I'm not actually editing the instructions. :)
1. install [virtualenv](http://www.virtualenv.org)
1. create a virtual environment (perhaps in this directory): `virtualenv _ve`
1. activate the virtualenv: `. _ve/bin/activate`
1. install pip requirements: `pip install -r requirements.txt` (you may be required to upgrade your version of setuptools via the provided command)

## running (OS X)

    /Applications/Kivy.app/Contents/Resources/script racecapture.py

## Preparing to build installers (OSX)

1. activate the virtualenv: `_ve/bin/activate`
1. Install [PYInstaller] (http://www.pyinstaller.org) `pip install pyinstaller`
1. Try `pyinstaller --version` - should return "2.1" or similar
1. Open `/Applications/Kivy.app/Contents/Resources/lib/sitepackages/pygments/lexers/__init__.py` and add a line "from pygments.lexers.agile import PythonLexer" near the top. Yes, I know this is bad form. It's to fix an error ("AttributeError: 'module' object has no attribute 'PythonLexer'") where PyInstaller failed to find an import and I couldn't work out how to force it. -CLR 2014-05-29

## Creating installer for current version (OSX)

1. activate the virtualenv: `_ve/bin/activate`
1. `cd install`
1.`./buildmacinstall.sh`
1. The install builder creates a .dmg file in the dist folder that is intended to be the app for distribution. However, it doesn't work right now ("Application damaged or incomplete") and I'm not sure why. -CLR 2014-05-29

## installation (Win7)

1. download [Kivy](http://kivy.org/#download) (current is 1.8.0) - remember to get the py2.7 version
1. follow Kivy install instructions
1. `my_kivy_install_folder\kivy.bat` (sets path variables for current shell)
1. install [virtualenv]: `pip install virtualenv`
1. create a virtual environment (perhaps in this directory): `virtualenv _ve`
1. activate the virtualenv: `_ve\Scripts\activate`
1. install pip requirements: `pip install -r requirements.txt` (if this crashes you installed the py3.3 version of Kivy, doughball)

## running (Win7)

    kivy racecapture.py
	Note: If you get an error doing this ("cannot find text provider") then you might be like me and be unable to run RaceCapture inside a virtualenv for some reason. Try performing the above steps without the virtual env part.

## dev installation (Eclipse on Win7, but probably applies to Eclipse on any platform)

1. Do the "installation" instructions above
1. download [Eclipse] (https://www.eclipse.org/downloads/)
1. Install [PyDev for Eclipse] (http://pydev.org/manual_101_install.html) and use auto-config
1. In Window..Preferences..PyDev..Interpreter..Python Interpreter..Environment, add a variable "PATH" with value c:\kivy;c:\kivy\Python;c:\kivy\gstreamer\bin;c:\kivy\MinGW\bin;%PATH%
1. In Window..Preferences..PyDev..Interpreter..Python Interpreter..Forced Builtins, add "kivy" to the list
1. Make a new project in Eclipse, select "PyDev project" and specify the folder with Racecapture in it
1. Right-click the project...Properties...pyDev-PYTHONPATH...External Libraries - Add source folder, add my_kivy_install_folder\kivy
1. Run the project

## Preparing to build installers (Win7)

1. `kivy` (to get paths set up)
1. activate the virtualenv: `_ve\Scripts\activate`
1. Install [PYInstaller] (http://www.pyinstaller.org) `pip install pyinstaller`
1. Install [PyWin32] (http://sourceforge.net/projects/pywin32/files/) `pip install pywin`
1. Try `pyinstaller --version` - should return "2.1" or similar
1. Install [nullsoft scriptable install system] (http://nsis.sourceforge.net/Download) stable version (currently 2.46)
1. Open (from your Kivy folder) `\Python27\Lib\site-packages\pygments\lexers\__init__.py` and add a line "from pygments.lexers.agile import PythonLexer" near the top. Yes, I know this is bad form. It's to fix an error ("AttributeError: 'module' object has no attribute 'PythonLexer'") where PyInstaller failed to find an import and I couldn't work out how to force it. You would think that this could be forced by creating a hook-pygments.lexers.py file with hiddenimports = ['agile'] in it, but you'd be wrong as this file already exists in the default PyInstaller hooks and it doesn't fix this problem. -CLR 2014-05-29

## Creating installer for current version (Win7)

1. Go into RaceCapture_App folder
1. activate the virtualenv: `_ve\Scripts\activate`
1. `kivy` (to get paths set up)
1. cd install
1. `buildwininstall.bat` - deletes old build folders, builds installer and tries to run the package (to run manually use `dist\racecapture\racecapture`)

## dev installation (Linux - Ubuntu)

1. sudo add-apt-repository ppa:kivy-team/kivy
1. sudo apt-get update
1. sudo apt-get install python-kivy
1. install [virtualenv]: `pip install virtualenv`
1. create a virtual environment (perhaps in this directory): `virtualenv _ve`
1. activate the virtualenv: `. _ve/bin/activate`
1. install pip requirements: `pip install -r requirements.txt` (you may be required to upgrade your version of setuptools via the provided command)

## running (Linux - Ubuntu)

    python racecapture.py

