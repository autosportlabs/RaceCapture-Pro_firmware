RaceCapture App
===============

Next Gen version of the RaceCapture App.

Dependencies:
* Python 2.7.x
* Kivy 1.8.x
* Pyserial 2.6.x
* Graph (via kivy-garden via python-pip)

to run:

    python racecapture.py

## dev installation (OS X)

1. download [Kivy](http://kivy.org/#download) (current is 1.8.0)
2. follow Kivy install instructions
- Chris Rae: I found step 3 a bit easier installing pip (`sudo easy_install pip`) and then virtualenv (`sudo pip install virtualenv`). But I don't know about Macs so I'm not actually editing the instructions. :)
3. install [virtualenv](http://www.virtualenv.org)
4. create a virtual environment (perhaps in this directory): `virtualenv _ve`
5. activate the virtualenv: `. _ve/bin/activate`
6. install pip requirements: `pip install -r requirements.txt` (you may be required to upgrade your version of setuptools via the provided command)
7. install graph via garden: `garden install graph`

## running (OS X)

    /Applications/Kivy.app/Contents/Resources/script racecapture.py

## Preparing to build installers (OSX)

1. activate the virtualenv: `_ve/bin/activate`
2. Install [PYInstaller] (http://www.pyinstaller.org) `pip install pyinstaller`
3. Try `pyinstaller --version` - should return "2.1" or similar
4. Open `/Applications/Kivy.app/Contents/Resources/lib/sitepackages/pygments/lexers/__init__.py` and add a line "from pygments.lexers.agile import PythonLexer" near the top. Yes, I know this is bad form. It's to fix an error ("AttributeError: 'module' object has no attribute 'PythonLexer'") where PyInstaller failed to find an import and I couldn't work out how to force it. -CLR 2014-05-29

## Creating installer for current version (OSX)

1. activate the virtualenv: `_ve/bin/activate`
2.`./buildmacinstall.sh`
3. The install builder creates a .dmg file in the dist folder that is intended to be the app for distribution. However, it doesn't work right now ("Application damaged or incomplete") and I'm not sure why. -CLR 2014-05-29

## installation (Win7)

1. download [Kivy](http://kivy.org/#download) (current is 1.8.0) - remember to get the py2.7 version
2. follow Kivy install instructions
3. Set path variables for current shell: `my_kivy_install_folder\kivy.bat`
4. install [virtualenv]: `pip install virtualenv`
5. create a virtual environment (perhaps in this directory): `virtualenv _ve`
6. activate the virtualenv: `_ve\Scripts\activate`
7. install pip requirements: `pip install -r requirements.txt`
8. install graph via garden: `garden install graph`

## running (Win7)

    kivy racecapture.py

## dev installation (Win7)

1. Do the "installation" instructions above
2. download [Eclipse] (https://www.eclipse.org/downloads/)
3. Set path variables for current shell: `my_kivy_install_folder\kivy.bat`
4. `set path`
5. Right click My Computer...System Properties...Advanced...Environment Variables...System Variables and add the extra folders that were added to the path to your system path
6. Install [PyDev for Eclipse] (http://pydev.org/manual_101_install.html) and use auto-config
7. Make a new project in Eclipse, select "PyDev project" and specify the folder with Racecapture in it
8. Right-click the project...Properties...pyDev-PYTHONPATH...External Libraries - Add source folder, add my_kivy_install_folder\kivy
9. Run the project

## Preparing to build installers (Win7)

1. activate the virtualenv: `_ve\Scripts\activate`
2. Install [PYInstaller] (http://www.pyinstaller.org) `pip install pyinstaller`
3. Install [PyWin32] (http://sourceforge.net/projects/pywin32/files/) `pip install pywin`
4. Try `pyinstaller --version` - should return "2.1" or similar
5. Install [nullsoft scriptable install system] (http://nsis.sourceforge.net/Download) stable version (currently 2.46)
6. Open (from your Kivy folder) `\Python27\Lib\site-packages\pygments\lexers\__init__.py` and add a line "from pygments.lexers.agile import PythonLexer" near the top. Yes, I know this is bad form. It's to fix an error ("AttributeError: 'module' object has no attribute 'PythonLexer'") where PyInstaller failed to find an import and I couldn't work out how to force it. -CLR 2014-05-29

## Creating installer for current version (Win7)

1. Go into RaceCapture_App folder
2. `kivy` (to get paths set up)
3. cd install
4. `buildwininstall.bat` - deletes old build folders, builds installer and tries to run the package (to run manually use `dist\racecapture\racecapture`)
