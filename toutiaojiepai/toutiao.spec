# -*- mode: python -*-

block_cipher = None


a = Analysis(['toutiao.py'],
             pathex=['D:\\×ÊÁÏ\\º®¼ÙÊµ¼ù\\python¿â\\toutiaojiepai'],
             binaries=[],
             datas=[],
             hiddenimports=[],
             hookspath=[],
             runtime_hooks=[],
             excludes=[],
             win_no_prefer_redirects=False,
             win_private_assemblies=False,
             cipher=block_cipher)
pyz = PYZ(a.pure, a.zipped_data,
             cipher=block_cipher)
exe = EXE(pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          name='toutiao',
          debug=False,
          strip=False,
          upx=True,
          runtime_tmpdir=None,
          console=True )
