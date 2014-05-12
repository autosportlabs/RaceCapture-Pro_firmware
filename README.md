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

1. Install [PYInstaller] (http://www.pyinstaller.org) `pip install pyinstaller`
2. Install [PyWin32] (http://sourceforge.net/projects/pywin32/files/) `pip install pywin32`
3. Try `pyinstaller --version` - should return "2.1" or similar

## Creating installer for current version

1. Go into RaceCapture_App folder
2. `kivy`
3. `pyinstaller -y racecapture.spec` (may need to run several times - first few times get weird missing file reports)
4. `dist\racecapture\racecapture`
5. Observe "ImportError: No module named decimal" message. Sob sadly. According to [this] (http://mayankjohri.wordpress.com/2008/05/08/faq-resolving-the-importerror-no-module-named-decimal-error/) the problem can be solved by importing Decimal in your own code (it may be because Kivy garden is doing some too-clever import stuff). However, it kicks the can down the road again to a new error ("AttributeError: 'module' object has no attribute 'PythonLexer'") so I haven't done it.