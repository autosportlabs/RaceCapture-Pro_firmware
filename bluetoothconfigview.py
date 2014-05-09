import kivy
kivy.require('1.8.0')
from kivy.app import Builder
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button

from settingsview import SettingsView, SettingsSwitch, SettingsButton
from separator import HLineSeparator
from valuefield import ValueField
from utils import *

Builder.load_file('bluetoothconfigview.kv')

class BluetoothTelemetryView(BoxLayout):
    def __init__(self, **kwargs):
        super(BluetoothTelemetryView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        btConfig = kvFind(self, 'rcid', 'btconfig')
        btConfig.bind(on_setting=self.on_bt_configure)
        btConfig.setControl(SettingsButton(text='Configure'))
        
        btEnable = kvFind(self, 'rcid', 'btenable') 
        btEnable.bind(on_setting=self.on_bt_enable)
        btEnable.setControl(SettingsSwitch())
        

    def on_bt_configure(self, instance, value):
        pass
        
    def on_bt_enable(self, instance, value):
        pass
        
    def on_config_updated(self, rcpCfg):
        pass
