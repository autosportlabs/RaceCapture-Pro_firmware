import kivy
kivy.require('1.8.0')
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from kivy.properties import StringProperty, ListProperty

from fieldlabel import FieldLabel
from utils import kvFind
import mainfonts

Builder.load_file('autosportlabs/racecapture/menu/mainmenu.kv')

class MainMenuItem(BoxLayout):
    rcid = None
    icon = StringProperty('')
    description = StringProperty('')

    def __init__(self, **kwargs):
        super(MainMenuItem, self).__init__(**kwargs)
        self.bind(icon = self.on_icon_text)
        self.bind(description = self.on_description_text)
        rcid = kwargs.get('rcid', None)
        self.register_event_type('on_main_menu_item_selected')

    def on_main_menu_item_selected(self, value):
        pass

    def on_icon_text(self, instance, value):
        help = kvFind(self, 'rcid', 'icon')
        help.text = value

    def on_description_text(self, instance, value):
        label = kvFind(self, 'rcid', 'desc')
        label.text = value
    
    def on_touch_up(self, touch):
        if self.collide_point(*touch.pos):
            self.dispatch('on_main_menu_item_selected', self.rcid)
            return True
        return super(MainMenuItem, self).on_touch_up(touch)

class MainMenu(BoxLayout):
        
    def __init__(self, **kwargs):
        super(MainMenu, self).__init__(**kwargs)
        self.register_event_type('on_main_menu_item')
        
    def on_main_menu_item(self, value):
        pass
    
    def on_main_menu_item_selected(self, instance, value):
        self.dispatch('on_main_menu_item', value)


