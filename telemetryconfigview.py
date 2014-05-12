import kivy
kivy.require('1.8.0')
from kivy.app import Builder
from kivy.uix.boxlayout import BoxLayout
import json

from settingsview import SettingsView, SettingsTextField, SettingsSwitch
from separator import HLineSeparator
from valuefield import ValueField
from utils import *

Builder.load_file('telemetryconfigview.kv')

class TelemetryConfigView(BoxLayout):
    connectivityConfig = None
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
        if self.connectivityConfig:
            self.connectivityConfig.telemetryConfig.deviceId = value
    
    def on_bg_stream(self, instance, value):
        if self.connectivityConfig:
            self.connectivityConfig.connectionModes.backgroundStreaming = value
    
    def on_config_updated(self, rcpCfg):
        connectivityConfig = rcpCfg.connectivityConfig
        kvFind(self, 'rcid', 'bgStream').setValue(connectivityConfig.telemetryConfig.backgroundStreaming)
        kvFind(self, 'rcid', 'deviceId').setValue(connectivityConfig.telemetryConfig.deviceId)
        self.connectivityConfig = connectivityConfig
        
        
        
        
    