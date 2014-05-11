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
    customApnLabel = 'Custom APN'
    apnSpinner = None
    cellProviderInfo = None
    connectivityConfig = None
    apnHostField = None
    apnUserField = None
    apnPassField = None
    
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
    
        self.apnHostField = kvFind(self, 'rcid', 'apnHost')
        self.apnUserField = kvFind(self, 'rcid', 'apnUser')
        self.apnPassField = kvFind(self, 'rcid', 'apnPass')

    def setCustomApnFieldsDisabled(self, disabled):
        self.apnHostField.disabled = disabled
        self.apnUserField.disabled = disabled
        self.apnPassField.disabled = disabled
        
    def on_cell_provider(self, instance, value):
        apnSetting = self.getApnSettingByName(value)
        knownProvider = False
        if apnSetting:
            self.apnHostField.text = apnSetting['apn_host']
            self.apnUserField.text = apnSetting['apn_user']
            self.apnPassField.text = apnSetting['apn_pass']
            knownProvider = True
            
        self.update_controls_state()
        self.setCustomApnFieldsDisabled(knownProvider)
        
    
    def on_telemetry_enable(self, instance, value):
        pass
    
    def on_bt_configure(self, instance, value):
        pass
        
    def on_bt_enable(self, instance, value):
        pass
    
    def on_apn_host(self, instance, value):
        if self.connectivityConfig:
            self.connectivityConfig.cellConfig.apnHost = value
            
    def on_apn_user(self, instance, value):
        if self.connectivityConfig:
            self.connectivityConfig.cellConfig.apnUser = value
        
    def on_apn_pass(self, instance, value):
        if self.connectivityConfig:
            self.connectivityConfig.cellConfig.apnPass = value
        
    def getApnSettingByName(self, name):
        providers = self.cellProviderInfo['cellProviders']
        for apnName in providers:
            if apnName == name:
                return providers[apnName]
        return None
    
    def loadApnSettingsSpinner(self, spinner):
        try:
            json_data = open('resource/settings/cell_providers.json')
            cellProviderInfo = json.load(json_data)
            apnMap = {}
            apnMap['custom'] = self.customApnLabel

            for name in cellProviderInfo['cellProviders']:
                apnMap[name] = name
                    
            spinner.setValueMap(apnMap, self.customApnLabel)
            self.cellProviderInfo = cellProviderInfo
        except Exception as detail:
            print('Error loading cell providers ' + str(detail))
        
    def update_controls_state(self):
        if self.connectivityConfig:
            cellProviderInfo = self.cellProviderInfo
            existingApnName = self.customApnLabel
            customFieldsDisabled = False
            cellConfig = self.connectivityConfig.cellConfig
            providers = cellProviderInfo['cellProviders']
            for name in providers:
                apnInfo = providers[name]
                if  apnInfo['apn_host'] == cellConfig.apnHost and apnInfo['apn_user'] == cellConfig.apnUser and apnInfo['apn_pass'] == cellConfig.apnPass:
                    existingApnName = name
                    customFieldsDisabled = True
                    break
            self.setCustomApnFieldsDisabled(customFieldsDisabled)
            return existingApnName
                
    def on_config_updated(self, rcpCfg):
        connectivityConfig = rcpCfg.connectivityConfig
        
        bluetoothEnabled = False
        telemetryEnabled = False
        kvFind(self, 'rcid', 'btenable').setValue(bluetoothEnabled)
        kvFind(self, 'rcid', 'telemetryenable').setValue(telemetryEnabled)
        
        self.apnHostField.text = connectivityConfig.cellConfig.apnHost
        self.apnUserField.text = connectivityConfig.cellConfig.apnUser
        self.apnPassField.text = connectivityConfig.cellConfig.apnPass
        
        self.connectivityConfig = connectivityConfig
            
        existingApnName = self.update_controls_state()
        if existingApnName:
            self.apnSpinner.text = existingApnName


        