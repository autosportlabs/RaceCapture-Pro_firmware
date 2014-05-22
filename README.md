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
3. install [virtualenv](http://www.virtualenv.org)
4. create a virtual environment (perhaps in this directory): `virtualenv _ve`
5. activate the virtualenv: `. _ve/bin/activate`
6. install pip requirements: `pip install -r requirements.txt`
7. install graph via garden: `garden install graph`

## running (OS X)

    /Applications/Kivy.app/Contents/Resources/script racecapture.py

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

## Building installers

1. activate the virtualenv: `_ve\Scripts\activate`
2. Install [PYInstaller] (http://www.pyinstaller.org) `pip install pyinstaller`
3. Install [PyWin32] (http://sourceforge.net/projects/pywin32/files/) `pip install pywin`
4. Try `pyinstaller --version` - should return "2.1" or similar

## Creating installer for current version

1. Go into RaceCapture_App folder
2. `kivy` (to get paths set up)
3. `pyinstaller -y racecapture.spec` (if you get weird file permissions errors on subsequent builds you may have to delete the 'build' folder and try it again)
4. `dist\racecapture\racecapture`
5. Observe error ("AttributeError: 'module' object has no attribute 'PythonLexer'"). This appears to be because the KivyLexer import in scriptview.py doesn't get bundled in the installer. If you comment out attach_node('Lua Script', n, LuaScriptingView()) in racecapture.py then the app runs fine.