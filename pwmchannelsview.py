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
            
            editor = kvquery(self, id='pwm' + str(i)).next()
            pwmChannel = pwmCfg.channels[i]
            
            channelSpinner = kvquery(editor, rcid='chan').next()
            channelSpinner.setValue(self.channels.getNameForId(pwmChannel.channelId))

            sampleRateSpinner = kvquery(editor, rcid='sr').next()
            sampleRateSpinner.setValue(pwmChannel.sampleRate)
            
            voltageScalingField = kvquery(editor, rcid='vScal').next()
            voltageScalingField.text =  '{:.3g}'.format(pwmChannel.voltageScaling)
            
            startupDutyCycle = kvquery(editor, rcid='dutyCycle').next()
            startupDutyCycle.text = str(pwmChannel.startupDutyCycle)
            
            startupPeriod = kvquery(editor, rcid='period').next()
            startupPeriod.text = str(pwmChannel.startupPeriod)
            
            outputModeSpinner = kvquery(editor, rcid='outputMode').next()
            outputModeSpinner.setFromValue(pwmChannel.outputMode)
            
            loggingModeSpinner = kvquery(editor, rcid='loggingMode').next()
            loggingModeSpinner.setFromValue(pwmChannel.loggingMode)
            
            
