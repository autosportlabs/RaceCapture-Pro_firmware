import kivy
kivy.require('1.8.0')
from kivy.app import Builder
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
import json

from settingsview import SettingsView, SettingsTextField, SettingsSwitch
from separator import HLineSeparator
from valuefield import ValueField
from utils import *

Builder.load_file('telemetryconfigview.kv')

class TelemetryConfigView(BoxLayout):
    
    def __init__(self, **kwargs):    
        super(TelemetryConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
    
        btEnable = kvFind(self, 'rcid', 'deviceId') 
        btEnable.bind(on_setting=self.on_device_id)
        btEnable.setControl(SettingsTextField())
        
        bgStream = kvFind(self, 'rcid', 'bgStream')
        bgStream.bind(on_setting=self.on_bg_stream)
        bgStream.setControl(SettingsSwitch())
        
    def on_device_id(self, instance, value):
        pass
    
    def on_bg_stream(self, instance, value):
        pass
    
    def on_config_updated(self, rcpCfg):
        pass
    