# -*- mode: python -*-
# RaceCapture PyInstaller spec file. This builds a folder with the necessary files to
# run RaceCapture - it also includes *all* kv files and *all* ttf files from the
# RaceCapture folder and children. If we add kv or ttf files which are not to go in the distribution
# (or add any other file types that are required) then these will need to be manually
# enumerated in this file. CLR 2014-05-26
from kivy.tools.packaging.pyinstaller_hooks import install_hooks
install_hooks(globals(), ['hooks'])
def addKVs():
    allFiles = Tree('..//')
    extraDatas = []
    for file in allFiles:
        if file[0].endswith('.kv') | file[0].endswith('.ttf') | file[0].startswith('resource/'):
            print "Adding datafile: " + file[0]
            extraDatas.append(file)
    return extraDatas

a = Analysis(['..//racecapture.py'],
             pathex=['..//'],
             hiddenimports=[],
             runtime_hooks=None)
a.datas += addKVs()
pyz = PYZ(a.pure)
exe = EXE(pyz,
          a.scripts,
          [('v', None, 'OPTION')],
          exclude_binaries=True,
          name='racecapture' + ('.exe' if sys.platform == 'win32' else ''),
          debug=False,
          strip=None,
          upx=True,
          console=True )

coll = COLLECT(exe,
               a.binaries,
               a.zipfiles,
               a.datas,
               strip=None,
               upx=True,
               name='racecapture')

if sys.platform == 'darwin':
     app = BUNDLE(coll,
                  name='racecapture.app',
                  icon='resource//race_capture_icon_large.icns')
