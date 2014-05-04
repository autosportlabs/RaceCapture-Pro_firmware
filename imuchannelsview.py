import kivy
kivy.require('1.8.0')

from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from kivy.uix.spinner import Spinner
from mappedspinner import MappedSpinner
from rcpconfig import *
from utils import *

Builder.load_file('imuchannelsview.kv')

class OrientationSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(OrientationSpinner, self).__init__(**kwargs)
        self.setValueMap({0:'Disabled', 1:'Normal', 2:'Inverted'}, 'Disabled')

class ImuMappingSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(ImuMappingSpinner, self).__init__(**kwargs)

    def setImuType(self, imuType):
        if imuType == 'accel':
            self.setValueMap({0:'X', 1:'Y', 2:'Z'}, 'X')
        elif imuType == 'gyro':
            self.setValueMap({3:'Yaw', 4:'Pitch', 5:'Roll'}, 'Yaw')
        
class GyroChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(GyroChannelsView, self).__init__(**kwargs)

class AccelChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(AccelChannelsView, self).__init__(**kwargs)

class ImuChannel(BoxLayout):
    channelConfig = None
    channelLabels = []
    def __init__(self, **kwargs):
        super(ImuChannel, self).__init__(**kwargs)
        self.imu_id = kwargs.get('imu_id', None)
    
    def on_zero_value(self, instance, value):
        pass
    
    def on_orientation(self, instance, value):
        pass
    
    def on_mapping(self, instance, value):
        pass
    
    def on_mode(self, instance, value):
        pass
    
    def on_enabled(self, instance, value):
        disabled = not value
        kvFind(self, 'rcid', 'orientation').disabled = disabled
        kvFind(self, 'rcid', 'mapping').disabled = disabled
        kvFind(self, 'rcid', 'zeroValue').disabled = disabled
        if self.channelConfig:
            pass


    def on_config_updated(self, channelIndex, channelConfig, channelLabels):
        label = kvFind(self, 'rcid', 'label')
        label.text = channelLabels[channelIndex]

        enabled = kvFind(self, 'rcid', 'enabled')
        enabled.active = not channelConfig.mode == 0

        orientation = kvFind(self, 'rcid', 'orientation')
        orientation.setFromValue(channelConfig.mode)

        mapping = kvFind(self, 'rcid', 'mapping')
        if channelIndex in(IMU_ACCEL_CHANNEL_IDS):
            mapping.setImuType('accel')
        elif channelIndex in(IMU_GYRO_CHANNEL_IDS):
            mapping.setImuType('gyro')

        mapping.setFromValue(channelConfig.chan)
            
        zeroValue = kvFind(self, 'rcid', 'zeroValue')
        zeroValue.text = str(channelConfig.zeroValue)
        self.channelConfig = channelConfig
        self.channelLabels = channelLabels
        
class ImuChannelsView(BoxLayout):
    editors = []
    imuCfg = None
    channelLabels = {0:'X', 1:'Y', 2:'Z', 3:'Yaw'}
    def __init__(self, **kwargs):
        super(ImuChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')

        accelContainer = kvFind(self, 'rcid', 'ac')
        self.appendImuChannels(accelContainer, self.editors, IMU_ACCEL_CHANNEL_IDS)

        gyroContainer = kvFind(self, 'rcid', 'gc')
        self.appendImuChannels(gyroContainer, self.editors, IMU_GYRO_CHANNEL_IDS)
        
        kvFind(self, 'rcid', 'sr').bind(on_sample_rate = self.on_sample_rate)        
        
        
    def appendImuChannels(self, container, editors, ids):
        for i in ids:
            editor = ImuChannel(rcid='imu_chan_' + str(i))
            container.add_widget(editor)
            editors.append(editor)
        
    def on_sample_rate(self, instance, value):
        if self.imuCfg:
            for imuChannel in self.imuCfg.channels:
                imuChannel.sampleRate = value
            
    def on_config_updated(self, rcpCfg):
        imuCfg = rcpCfg.imuConfig
        channelCount = imuCfg.channelCount

        for i in range(channelCount):
            imuChannel = imuCfg.channels[i]
            editor = self.editors[i]
            editor.on_config_updated(i, imuChannel, self.channelLabels)
        
        self.imuCfg = imuCfg

