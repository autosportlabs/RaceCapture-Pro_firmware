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
from autosportlabs.racecapture.views.configuration.baseconfigview import BaseConfigView

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/timerchannelsview.kv')

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
    channelConfig = None
    channels = None
    def __init__(self, **kwargs):
        super(PulseChannel, self).__init__(**kwargs)
        kvFind(self, 'rcid', 'sr').bind(on_sample_rate = self.on_sample_rate)
        kvFind(self, 'rcid', 'chanId').bind(on_channel = self.on_channel)
        self.register_event_type('on_modified')
        self.register_event_type('on_modified')
    
    def on_modified(self, channelConfig):
        pass

    def on_channel(self, instance, value):
        if self.channelConfig:
            self.channelConfig.channelId = self.channels.getIdForName(value)
            self.channelConfig.stale = True
            self.dispatch('on_modified', self.channelConfig)
                                    
    def on_sample_rate(self, instance, value):
        if self.channelConfig:
            self.channelConfig.sampleRate = value
            self.channelConfig.stale = True
            self.dispatch('on_modified', self.channelConfig)
            
    def on_pulse_per_rev(self, instance, value):
        if self.channelConfig:
            self.channelConfig.pulsePerRev = int(value)
            self.channelConfig.stale = True
            self.dispatch('on_modified', self.channelConfig)
                        
    def on_mode(self, instance, value):
        if self.channelConfig:
            self.channelConfig.mode = int(instance.getValueFromKey(value))
            self.channelConfig.stale = True
            self.dispatch('on_modified', self.channelConfig)
                        
    def on_divider(self, instance, value):
        if self.channelConfig:
            self.channelConfig.divider = int(value)
            self.channelConfig.stale = True
            self.dispatch('on_modified', self.channelConfig)
                            
    def on_config_updated(self, channelConfig, channels):
        sampleRateSpinner = kvFind(self, 'rcid', 'sr')
        sampleRateSpinner.setValue(channelConfig.sampleRate)
    
        channelSpinner = kvFind(self, 'rcid', 'chanId')
        channelSpinner.setValue(channels.getNameForId(channelConfig.channelId))
        
        modeSpinner = kvFind(self, 'rcid', 'mode')
        modeSpinner.setFromValue(channelConfig.mode)
        
        dividerSpinner = kvFind(self, 'rcid', 'divider')
        dividerSpinner.setFromValue(channelConfig.divider)
        
        pulsePerRevSpinner = kvFind(self, 'rcid', 'ppr')
        pulsePerRevSpinner.setFromValue(channelConfig.pulsePerRev)
        
        self.channelConfig = channelConfig
        self.channels = channels

class PulseChannelsView(BaseConfigView):
    editors = []
    channels = None
    accordion = None
    timerCfg = None
    def __init__(self, **kwargs):
        super(PulseChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        self.channelCount = kwargs['channelCount']
        
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=110 * 3)
        self.accordion = accordion
        
        editors = []
        # add button into that grid
        for i in range(self.channelCount):
            channel = AccordionItem(title='Pulse Input ' + str(i + 1))
            editor = PulseChannel(id='timer' + str(i))
            channel.add_widget(editor)
            editor.bind(on_modified=self.on_modified)            
            accordion.add_widget(channel)
            editors.append(editor)
    
        accordion.select(accordion.children[-1])
    
        self.editors = editors
        #create a scroll view, with a size < size of the grid
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

    def on_modified(self, instance, channelConfig):
        self.setAccordionItemTitle(self.accordion, self.timerCfg.channels, channelConfig)
        super(PulseChannelsView, self).on_modified(self, instance, channelConfig)

    def on_config_updated(self, rcpCfg):
        timerCfg = rcpCfg.timerConfig
        channels = rcpCfg.channels
        self.channels = rcpCfg.channels
        self.timerCfg = timerCfg
        
        channelCount = timerCfg.channelCount
        for i in range(channelCount):
            editor = self.editors[i]
            timerChannel = timerCfg.channels[i]
            self.setAccordionItemTitle(self.accordion, timerCfg.channels, timerChannel)            
            editor.on_config_updated(timerChannel, channels)
