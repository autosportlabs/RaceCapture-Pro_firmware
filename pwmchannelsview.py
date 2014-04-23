import kivy
kivy.require('1.8.0')

from kivy.uix.label import Label
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from fieldlabel import FieldLabel
from mappedspinner import MappedSpinner
from utils import *

Builder.load_file('pwmchannelsview.kv')
    
class PwmOutputModeSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(PwmOutputModeSpinner, self).__init__(**kwargs)
        self.setValueMap({0:'Analog', 1:'Frequency'}, 'Frequency')

class PwmLoggingModeSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(PwmLoggingModeSpinner, self).__init__(**kwargs)
        self.setValueMap({0:'Period', 1:'Duty Cycle', 2:'Volts'}, 'Duty Cycle')
    
class AnalogPulseOutputChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(AnalogPulseOutputChannel, self).__init__(**kwargs)
    
class AnalogPulseOutputChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        self.register_event_type('on_config_updated')
        self.channelCount = kwargs['channelCount']
        self.channels = kwargs['channels']
        
        super(AnalogPulseOutputChannelsView, self).__init__(**kwargs)
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=100 * self.channelCount)
    
        # add button into that grid
        for i in range(self.channelCount):
            channel = AccordionItem(title='Pulse / Analog Output ' + str(i + 1))
            editor = AnalogPulseOutputChannel(id='pwm' + str(i))
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

    def on_config_updated(self, rcpCfg):
        pwmCfg = rcpCfg.pwmConfig
        channelCount = pwmCfg.channelCount

        for i in range(channelCount):
            
            editor = kvFind(self, 'id', 'pwm' + str(i))
            pwmChannel = pwmCfg.channels[i]
            
            channelSpinner = kvFind(editor, 'rcid', 'chan')
            channelSpinner.setValue(self.channels.getNameForId(pwmChannel.channelId))

            sampleRateSpinner = kvFind(editor, 'rcid', 'sr')
            sampleRateSpinner.setValue(pwmChannel.sampleRate)
            
            voltageScalingField = kvFind(editor, 'rcid', 'vScal')
            voltageScalingField.text =  '{:.3g}'.format(pwmChannel.voltageScaling)
            
            startupDutyCycle = kvFind(editor, 'rcid', 'dutyCycle')
            startupDutyCycle.text = str(pwmChannel.startupDutyCycle)
            
            startupPeriod = kvFind(editor, 'rcid', 'period')
            startupPeriod.text = str(pwmChannel.startupPeriod)
            
            outputModeSpinner = kvFind(editor, 'rcid', 'outputMode')
            outputModeSpinner.setFromValue(pwmChannel.outputMode)
            
            loggingModeSpinner = kvFind(editor, 'rcid', 'loggingMode')
            loggingModeSpinner.setFromValue(pwmChannel.loggingMode)
            
            
