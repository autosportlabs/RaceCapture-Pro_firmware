import kivy
kivy.require('1.8.0')
from utils import *
from kivy.uix.stacklayout import StackLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.progressbar import ProgressBar
from kivy.app import Builder
from iconbutton import IconButton

Builder.load_file('toolbarview.kv')

class ToolbarView(BoxLayout):
    progressBar = None
    def __init__(self, **kwargs):
        super(ToolbarView, self).__init__(**kwargs)
        self.register_event_type('on_main_menu')
        self.register_event_type('on_progress')
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
    def on_progress(self, value):
        if not self.progressBar:
            self.progressBar = kvFind(self, 'rcid', 'pbar')
        self.progressBar.value = value
        
    def on_main_menu(self, instance, *args):
        pass
        
    def mainMenu(self):
        self.dispatch('on_main_menu', None)
    
