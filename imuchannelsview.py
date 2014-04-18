import kivy
kivy.require('1.0.8')

from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from kivy.uix.spinner import Spinner
from rcpconfig import *
from utils import *

Builder.load_file('imuchannelsview.kv')

class OrientationSpinner(Spinner):
    def __init__(self, **kwargs):
        super(OrientationSpinner, self).__init__(**kwargs)
        self.values = ['Normal', 'Inverted']
        self.valueMappings = {0:'', 1:'Normal', 2:'Inverted'}
        
    def setFromValue(self, value):
        self.text = self.valueMappings.get(value, '')

class ImuMappingSpinner(Spinner):
    def __init__(self, **kwargs):
        super(ImuMappingSpinner, self).__init__(**kwargs)
        self.valueMappings = {0:'X', 1:'X', 2:'Y', 3:'Yaw', 4:'Pitch', 5:'Roll'}

        type = kwargs.get('type', 'accel')
        if type == 'accel':
            self.values = ['X', 'Y', 'Z']
        elif type == 'gyro':
            self.values = ['Yaw', 'Pitch', 'Roll']            
        
    def setFromValue(self, value):
        self.text = self.valueMappings.get(value, '')

class GyroChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(GyroChannelsView, self).__init__(**kwargs)

class AccelChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(AccelChannelsView, self).__init__(**kwargs)

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

        channelEditors = []
        
        self.appendImuChannels(accelContainer, channelEditors, IMU_ACCEL_CHANNEL_IDS)
        self.appendImuChannels(gyroContainer, channelEditors, IMU_GYRO_CHANNEL_IDS)
        self.channelEditors = channelEditors
        
        self.channelLabels = {0:'X', 1:'Y', 2:'Z', 3:'Yaw'}
        
    def appendImuChannels(self, container, channelEditors, ids):
        for i in ids:
            editor = ImuChannel(imu_id='imu_chan_' + str(i))
            container.add_widget(editor)
            channelEditors.append(editor)
        
        
    def update(self, rcpCfg):
        imuCfg = rcpCfg.imuConfig
        channelCount = imuCfg.channelCount

        for i in range(channelCount):
            imuChannel = imuCfg.channels[i]
            editor = self.channelEditors[i]
            label = kvquery(editor, imu_id='label').next()
            enabled = kvquery(editor, imu_id='enabled').next()
            orientation = kvquery(editor, imu_id='orientation').next()
            mapping = kvquery(editor, imu_id='mapping').next()
            zeroValue = kvquery(editor, imu_id='zeroValue').next()
        
            label.text = self.channelLabels[i]
            enabled.active = not imuChannel.mode == 0
            orientation.setFromValue(imuChannel.mode)
            mapping.setFromValue(imuChannel.chan)
            zeroValue.text = str(imuChannel.zeroValue)
            
        
        
        
    def on_config_updated(self, rcpCfg):
        self.update(rcpCfg)


