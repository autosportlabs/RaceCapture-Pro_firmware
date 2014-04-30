import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
from kivy.app import Builder

Builder.load_file('toolbarview.kv')

class ToolbarView(BoxLayout):

    def __init__(self, **kwargs):
        self.register_event_type('on_read_config')
        super(ToolbarView, self).__init__(**kwargs)
        self.rcp = kwargs.get('rcp', None)
        self.app = kwargs.get('app', None)

    def readConfig(self):
        self.dispatch('on_read_config', None)

    def on_read_config(self, instance, *args):
        pass

class ToolbarButton(Button):
    pass
