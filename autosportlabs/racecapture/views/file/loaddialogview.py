import kivy
kivy.require('1.8.0')

from kivy.properties import ObjectProperty
from kivy.uix.floatlayout import FloatLayout
from kivy.app import Builder
from autosportlabs.widgets.filebrowser import FileBrowser
from kivy.utils import platform
from os.path import dirname, expanduser, sep
from utils import kvFind

Builder.load_file('autosportlabs/racecapture/views/file/loaddialogview.kv')

class LoadDialog(FloatLayout):
    def __init__(self, **kwargs):
        super(LoadDialog, self).__init__(**kwargs)
        ok = kwargs.get('ok', None)
        cancel = kwargs.get('cancel', None)
        if platform() == 'win':
            user_path = dirname(expanduser('~')) + sep + 'Documents'
        else:
            user_path = expanduser('~') + sep + 'Documents'
            
        browser = kvFind(self, 'rcid', 'browser')
        browser.path = user_path
        browser.favorites = [(user_path, 'Documents')]
        browser.filters = kwargs.get('filters', ['*'])        
        if ok: browser.bind(on_success = ok)
        if cancel: browser.bind(on_canceled = cancel)
            