import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from iconbutton import IconButton

Builder.load_file('toolbarview.kv')

class ToolbarView(BoxLayout):

    def __init__(self, **kwargs):
        super(ToolbarView, self).__init__(**kwargs)
        self.register_event_type('on_main_menu')
        self.register_event_type('on_read_config')
        self.register_event_type('on_write_config')
        self.rcpComms = kwargs.get('rcpComms', None)
        self.app = kwargs.get('app', None)

    def on_main_menu(self, instance, *args):
        pass
    
    def on_read_config(self, instance, *args):
        pass
    
    def on_write_config(self, instance, *args):
        pass
    
    def mainMenu(self):
        self.dispatch('on_main_menu', None)
        
    def readConfig(self):
        self.dispatch('on_read_config', None)

    def writeConfig(self):
        self.dispatch('on_write_config', None)

