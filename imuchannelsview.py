import kivy
kivy.require('1.0.8')

from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from kivy.uix.spinner import Spinner
from rcpconfig import *
from utils import *

Builder.load_file('imuchannelsview.kv')

class GyroChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(GyroChannelsView, self).__init__(**kwargs)
        #self.id = 'gc'

class AccelChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(AccelChannelsView, self).__init__(**kwargs)
        #self.id = 'ac'

class AccelMappingSpinner(Spinner):
    def __init__(self, **kwargs):
        super(AccelMappingSpinner, self).__init__(**kwargs)
        self.values = ['X', 'Y', 'Z']

class GyroMappingSpinner(Spinner):
    def __init__(self, **kwargs):
        super(GyroMappingSpinner, self).__init__(**kwargs)
        self.values = ['Yaw']

class ImuChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(ImuChannel, self).__init__(**kwargs)
        self.imu_id = kwargs.get('imu_id', None)

class ImuChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(ImuChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        
        accelContainer = kvquery(self, imu_id='ac').next()
        gyroContainer = kvquery(self, imu_id='gc').next()

        self.appendImuChannels(accelContainer, IMU_ACCEL_CHANNEL_IDS)
        self.appendImuChannels(gyroContainer, IMU_GYRO_CHANNEL_IDS)
        
    def appendImuChannels(self, container, ids):
        for i in ids:
            editor = ImuChannel(imu_id='imu_chan_' + str(i))
            container.add_widget(editor)
        
        
    def on_config_updated(self, rcpCfg):
        print("config updated imu")
        pass

