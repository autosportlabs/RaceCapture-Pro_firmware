import kivy
kivy.require('1.8.0')
from kivy.app import Builder
from kivy.uix.boxlayout import BoxLayout
import json

from settingsview import SettingsView, SettingsTextField, SettingsSwitch
from autosportlabs.widgets.separator import HLineSeparator
from valuefield import ValueField
from utils import *
from autosportlabs.racecapture.views.configuration.baseconfigview import BaseConfigView

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/telemetryconfigview.kv')

class TelemetryConfigView(BaseConfigView):
    connectivityConfig = None
    def __init__(self, **kwargs):    
        super(TelemetryConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
    
        deviceId = kvFind(self, 'rcid', 'deviceId') 
        deviceId.bind(on_setting=self.on_device_id)
        deviceId.setControl(SettingsTextField())
        
        bgStream = kvFind(self, 'rcid', 'bgStream')
        bgStream.bind(on_setting=self.on_bg_stream)
        bgStream.setControl(SettingsSwitch())
        
    def on_device_id(self, instance, value):
        if self.connectivityConfig:
            self.connectivityConfig.telemetryConfig.deviceId = value
            self.connectivityConfig.stale = True
            self.dispatch('on_modified')
                
    def on_bg_stream(self, instance, value):
        if self.connectivityConfig:
            self.connectivityConfig.telemetryConfig.backgroundStreaming = value
            self.connectivityConfig.stale = True
            self.dispatch('on_modified')
                
    def on_config_updated(self, rcpCfg):
        connectivityConfig = rcpCfg.connectivityConfig
        kvFind(self, 'rcid', 'bgStream').setValue(connectivityConfig.telemetryConfig.backgroundStreaming)
        kvFind(self, 'rcid', 'deviceId').setValue(connectivityConfig.telemetryConfig.deviceId)
        self.connectivityConfig = connectivityConfig
        
        
        
        
    