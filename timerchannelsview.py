import kivy
kivy.require('1.8.0')

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.uix.spinner import Spinner
from kivy.app import Builder
from mappedspinner import MappedSpinner
from rcpconfig import *
from utils import *

Builder.load_file('timerchannelsview.kv')

class TimerModeSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(TimerModeSpinner, self).__init__(**kwargs)
        self.setValueMap({0:'RPM', 1:'Frequency', 2:'Period (ms)', 3:'Period (us)'}, 'RPM')
            
class DividerSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(DividerSpinner, self).__init__(**kwargs)
        self.setValueMap({2:'2', 8:'8', 32:'32', 128:'128', 1024:'1024'}, '128')
    
class PulsePerRevSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(PulsePerRevSpinner, self).__init__(**kwargs)
        valueMap = {}
        for i in range (1, 64):
            valueMap[i] = str(i)
        self.setValueMap(valueMap, '1');
    
class PulseChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(PulseChannel, self).__init__(**kwargs)

class PulseChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(PulseChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        
        self.channelCount = kwargs['channelCount']
        self.channels = kwargs['channels']
        
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=110 * 3)
    
        # add button into that grid
        for i in range(self.channelCount):
            channel = AccordionItem(title='Pulse Input ' + str(i + 1))
            editor = PulseChannel(id='timer' + str(i))
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

    def on_config_updated(self, rcpCfg):
        timerCfg = rcpCfg.timerConfig
        channelCount = timerCfg.channelCount
        
        for i in range(channelCount):
            timerChannel = timerCfg.channels[i]
            editor = kvFind(self, 'id', 'timer' + str(i))

            sampleRateSpinner = kvFind(editor, 'rcid', 'sr')
            sampleRateSpinner.setValue(timerChannel.sampleRate)

            channelSpinner = kvFind(editor, 'rcid', 'chanId')
            channelSpinner.setValue(self.channels.getNameForId(timerChannel.channelId))
            
            modeSpinner = kvFind(editor, 'rcid', 'mode')
            modeSpinner.setFromValue(timerChannel.mode)
            
            dividerSpinner = kvFind(editor, 'rcid', 'divider')
            dividerSpinner.setFromValue(timerChannel.divider)
            
            pulsePerRevSpinner = kvFind(editor, 'rcid', 'ppr')
            pulsePerRevSpinner.setFromValue(timerChannel.pulsePerRev)
        
