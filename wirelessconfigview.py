import kivy
kivy.require('1.8.0')
from kivy.app import Builder
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button
import json

from settingsview import SettingsView, SettingsSwitch, SettingsButton, SettingsMappedSpinner
from separator import HLineSeparator
from valuefield import ValueField
from utils import *

Builder.load_file('wirelessconfigview.kv')

class WirelessConfigView(BoxLayout):
    apnSpinner = None
    apnSettings = None
    def __init__(self, **kwargs):
        super(WirelessConfigView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')

        btEnable = kvFind(self, 'rcid', 'btenable') 
        btEnable.bind(on_setting=self.on_bt_enable)
        btEnable.setControl(SettingsSwitch())
        
        btConfig = kvFind(self, 'rcid', 'btconfig')
        btConfig.bind(on_setting=self.on_bt_configure)
        btConfig.setControl(SettingsButton(text='Configure', disabled=True))
        
        telemetryEnable = kvFind(self, 'rcid', 'telemetryenable')
        telemetryEnable.bind(on_setting=self.on_telemetry_enable)
        telemetryEnable.setControl(SettingsSwitch())

        telemetryEnable = kvFind(self, 'rcid', 'cellprovider')
        telemetryEnable.bind(on_setting=self.on_cell_provider)
        apnSpinner = SettingsMappedSpinner()
        self.loadApnSettingsSpinner(apnSpinner)
        self.apnSpinner = apnSpinner
        telemetryEnable.setControl(apnSpinner)

    def on_cell_provider(self, instance, value):
        print("cell provider")
        pass
    
    def on_telemetry_enable(self, instance, value):
        pass
    
    def on_bt_configure(self, instance, value):
        pass
        
    def on_bt_enable(self, instance, value):
        pass
        
    def loadApnSettingsSpinner(self, spinner):
        try:
            json_data = open('resource/settings/cell_providers.json')
            apnSettings = json.load(json_data)
            apnMap = {}
            defaultName = None
            for provider in apnSettings['cellProviders']:
                name = provider['name']
                key = provider['key']
                apnMap[key] = provider['name']
                if not defaultName:
                    defaultName = name
                    
                apnMap['custom']='Custom Setting'
            spinner.setValueMap(apnMap, defaultName)
        except Exception as detail:
            print('Error loading cell providers ' + str(detail))
        
    def on_config_updated(self, rcpCfg):
        pass
        