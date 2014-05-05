import kivy
kivy.require('1.8.0')

from kivy.uix.label import Label
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.switch import Switch
from fieldlabel import FieldLabel
from helplabel import HelpLabel
from kivy.app import Builder
from utils import *
from kivy.properties import StringProperty, BooleanProperty

Builder.load_file('settingsview.kv')

class SettingsView(BoxLayout):
    help_text = StringProperty('')
    label_text = StringProperty('')
    rcid = StringProperty('')
    def __init__(self, **kwargs):
        super(SettingsView, self).__init__(**kwargs)
        self.bind(help_text = self.on_help_text)
        self.bind(label_text = self.on_label_text)
        
    def on_help_text(self, instance, value):
        help = kvFind(self, 'rcid', 'helpLabel')
        help.text = value

    def on_label_text(self, instance, value):
        label = kvFind(self, 'rcid', 'fieldLabel')
        label.text = value
        
class SettingsViewSwitch(SettingsView):
    active = BooleanProperty()
    def __init__(self, **kwargs):
        super(SettingsViewSwitch, self).__init__(**kwargs)
        self.register_event_type('on_setting_active')

    def on_setting_active(self, value):
        pass
    
    def setValue(self, value):
        switch = kvFind(self, 'rcid', 'settingsCtrl')
        switch.active = value
    
    def on_switch_active(self, instance, value):
        self.dispatch('on_setting_active', value)
        
    
        
        

