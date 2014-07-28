import kivy
from valuefield import ValueField
kivy.require('1.8.0')

from kivy.uix.label import Label
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.switch import Switch
from kivy.uix.button import Button
from kivy.uix.spinner import Spinner
from fieldlabel import FieldLabel
from helplabel import HelpLabel
from kivy.app import Builder
from utils import *
from mappedspinner import MappedSpinner

from kivy.properties import StringProperty

Builder.load_file('settingsview.kv')

class SettingsButton(Button):
    def __init__(self, **kwargs):
        super(SettingsButton, self).__init__(**kwargs)
        self.register_event_type('on_control')

    def on_control(self, value):
        pass

    def setValue(self, value):
        self.active = value
    
    def on_button_active(self, value):
        self.dispatch('on_control', value)

class SettingsSwitch(Switch):
    def __init__(self, **kwargs):
        super(SettingsSwitch, self).__init__(**kwargs)
        self.register_event_type('on_control')

    def on_control(self, value):
        pass

    def setValue(self, value):
        self.active = value
    
    def on_switch_active(self, value):
        self.dispatch('on_control', value)

class SettingsMappedSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(SettingsMappedSpinner, self).__init__(**kwargs)
        self.register_event_type('on_control')

    def on_control(self, value):
        pass

    def setValue(self, value):
        self.setFromValue(value)
    
    def on_text(self, instance, value):
        self.dispatch('on_control', instance.getValueFromKey(value))

class SettingsTextField(ValueField):
    def __init__(self, **kwargs):
        super(SettingsTextField, self).__init__(**kwargs)
        self.register_event_type('on_control')

    def on_control(self, value):
        pass

    def setValue(self, value):
        self.text = value
    
    def on_text(self, instance, value):
        self.dispatch('on_control', value)
    
    
    
class SettingsView(AnchorLayout):
    help_text = StringProperty('')
    label_text = StringProperty('')
    control = None
    rcid = StringProperty('')
    def __init__(self, **kwargs):
        super(SettingsView, self).__init__(**kwargs)
        self.bind(help_text = self.on_help_text)
        self.bind(label_text = self.on_label_text)
        self.register_event_type('on_setting')     

    def on_setting(self, *args):
        pass
    
    def on_control(self, instance, value):
        self.dispatch('on_setting', value)
        pass

    def on_help_text(self, instance, value):
        help = kvFind(self, 'rcid', 'helpLabel')
        help.text = value

    def on_label_text(self, instance, value):
        label = kvFind(self, 'rcid', 'fieldLabel')
        label.text = value
        
    def setControl(self, widget):
        kvFind(self, 'rcid', 'control').add_widget(widget)
        widget.bind(on_control=self.on_control)
        self.control = widget
            
    def setValue(self, value):
        if self.control:
            self.control.setValue(value)
    
        
    
        
        

