import kivy
kivy.require('1.8.0')

from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from rcpconfig import *
from utils import *

Builder.load_file('gpschannelsview.kv')            
            
class GPSChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(GPSChannelsView, self).__init__(**kwargs)
#        Builder.load_file('analogchannelsview.kv')
        self.register_event_type('on_config_updated')

    def setCheckBox(self, gpsCfg, key, active):
        checkbox = kvquery(self, gps_id=key).next()
        checkbox.active = active
        
    def on_config_updated(self, rcpCfg):
        gpsConfig = rcpCfg.gpsConfig
        
        sampleRate = kvquery(self, gps_id='sr').next()
        sampleRate.setValue(gpsConfig.sampleRate)
        
        self.setCheckBox(gpsConfig, 'pos', gpsConfig.positionEnabled)
        self.setCheckBox(gpsConfig, 'speed', gpsConfig.speedEnabled)
        self.setCheckBox(gpsConfig, 'dist', gpsConfig.distanceEnabled)
        self.setCheckBox(gpsConfig, 'time', gpsConfig.timeEnabled)
        self.setCheckBox(gpsConfig, 'sats', gpsConfig.satellitesEnabled)
        