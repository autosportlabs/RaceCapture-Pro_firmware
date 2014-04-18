import kivy
kivy.require('1.0.8')

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.uix.spinner import Spinner
from kivy.app import Builder
from rcpconfig import *
from utils import *

Builder.load_file('timerchannelsview.kv')

class TimerModeSpinner(Spinner):
    def __init__(self, **kwargs):
        super(TimerModeSpinner, self).__init__(**kwargs)
        self.valueMappings = {0:'RPM', 1:'Frequency', 2:'Period(MS)', 3:'Period(us)'}
        
    def setFromValue(self, value):
        self.text = self.valueMappings.get(value, 'RPM')
    
class DividerSpinner(Spinner):
    def __init__(self, **kwargs):
        super(DividerSpinner, self).__init__(**kwargs)
        self.valueMappings = {2:'2', 8:'8', 32:'32', 128:'128', 1024:'1024'}
        
    def setFromValue(self, value):
        self.text = self.valueMappings.get(value, '128')
    
class PulsePerRevSpinner(Spinner):
    def __init__(self, **kwargs):
        super(PulsePerRevSpinner, self).__init__(**kwargs)
        valueMappings = {}
        for i in range (1, 64):
            valueMappings[i] = str(i)
            
        self.valueMappings = valueMappings
        
    def setFromValue(self, value):
        self.text = self.valueMappings.get(value, '1')
    
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
            editor = kvquery(self, id='timer' + str(i)).next()

            sampleRateSpinner = kvquery(editor, timer_id='sr').next()
            sampleRateSpinner.setValue(timerChannel.sampleRate)

            channelSpinner = kvquery(editor, timer_id='chan').next()
            channelSpinner.setValue(self.channels.getNameForId(timerChannel.channelId))
            
            modeSpinner = kvquery(editor, timer_id='mode').next()
            modeSpinner.setFromValue(timerChannel.mode)
            
            dividerSpinner = kvquery(editor, timer_id='divider').next()
            dividerSpinner.setFromValue(timerChannel.divider)
            
            pulsePerRevSpinner = kvquery(editor, timer_id='ppr').next()
            pulsePerRevSpinner.setFromValue(timerChannel.pulsePerRev)
        
