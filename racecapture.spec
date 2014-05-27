# -*- mode: python -*-
from kivy.tools.packaging.pyinstaller_hooks import install_hooks
install_hooks(globals(), ['hooks'])
a = Analysis(['racecapture.py'],
             pathex=['.'],
             hiddenimports=[],
             runtime_hooks=None)
pyz = PYZ(a.pure)
exe = EXE(pyz,
          a.scripts,
          exclude_binaries=True,
          name='racecapture.exe',
          debug=False,
          strip=None,
          upx=True,
          console=True )

# This should eventually not be the complete folder (e.g. we don't want pyinstall.bat)
coll = COLLECT(exe,
			   Tree('.'),
               a.binaries,
               a.zipfiles,
               a.datas,
               strip=None,
               upx=True,
               name='racecapture')
