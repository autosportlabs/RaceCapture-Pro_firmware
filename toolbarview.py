import kivy
kivy.require('1.8.0')
from kivy.uix.stacklayout import StackLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from iconbutton import IconButton

Builder.load_file('toolbarview.kv')

class ToolbarView(BoxLayout):

    def __init__(self, **kwargs):
        super(ToolbarView, self).__init__(**kwargs)
        self.register_event_type('on_main_menu')
        self.rcpComms = kwargs.get('rcpComms', None)
        self.app = kwargs.get('app', None)

    def on_main_menu(self, instance, *args):
        pass
        
    def mainMenu(self):
        self.dispatch('on_main_menu', None)
    
    
