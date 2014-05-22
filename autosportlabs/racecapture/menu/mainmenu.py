import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from kivy.properties import StringProperty

from fieldlabel import FieldLabel
from utils import kvFind

Builder.load_file('autosportlabs/racecapture/menu/mainmenu.kv')

class MainMenuItem(BoxLayout):
    icon = StringProperty('')
    description = StringProperty('')

    def __init__(self, **kwargs):
        super(MainMenuItem, self).__init__(**kwargs)
        self.bind(icon = self.on_icon_text)
        self.bind(description = self.on_description_text)

    def on_icon_text(self, instance, value):
        help = kvFind(self, 'rcid', 'icon')
        help.text = value

    def on_description_text(self, instance, value):
        label = kvFind(self, 'rcid', 'desc')
        label.text = value

class MainMenu(BoxLayout):
        
    def __init__(self, **kwargs):
        super(MainMenu, self).__init__(**kwargs)
        self.register_event_type('on_main_menu_item')
        
    def on_main_menu_item(self, instance, value):
        print('on main menu item')


