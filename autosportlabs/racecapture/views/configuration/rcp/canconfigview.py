import kivy
kivy.require('1.8.0')

from settingsview import SettingsMappedSpinner, SettingsSwitch
from mappedspinner import MappedSpinner
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from utils import *
from autosportlabs.racecapture.views.configuration.rcp.baseconfigview import BaseConfigView

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/canconfigview.kv')

class CANBaudRateSpinner(SettingsMappedSpinner):
    def __init__(self, **kwargs):    
        super(CANBaudRateSpinner, self).__init__(**kwargs)
        self.setValueMap({50000: '50K Baud', 125000: '125K Baud', 250000:'250K Baud', 500000:'500K Baud', 1000000:'1M Baud'}, '500K Baud')
    
class CANConfigView(BaseConfigView):
    canConfig = None
    def __init__(self, **kwargs):    
        super(CANConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
    
        btEnable = kvFind(self, 'rcid', 'canEnabled') 
        btEnable.bind(on_setting=self.on_can_enabled)
        btEnable.setControl(SettingsSwitch())
        
        canBaudRates = kvFind(self, 'rcid', 'canBaud')        
        canBaudRates.bind(on_setting=self.on_can_baud)
        baudRateSpinner = CANBaudRateSpinner()
        canBaudRates.setControl(baudRateSpinner)        
        
    def on_can_enabled(self, instance, value):
        if self.canConfig:
            self.canConfig.enabled = value
            self.canConfig.stale = True
            self.dispatch('on_modified')            
    
    def on_can_baud(self, instance, value):
        if self.canConfig:
            self.canConfig.baudRate = value
            self.canConfig.stale = True
            self.dispatch('on_modified')
    
    def on_config_updated(self, rcpCfg):
        canConfig = rcpCfg.canConfig
        kvFind(self, 'rcid', 'canEnabled').setValue(canConfig.enabled)
        kvFind(self, 'rcid', 'canBaud').setValue(canConfig.baudRate)
        self.canConfig = canConfig