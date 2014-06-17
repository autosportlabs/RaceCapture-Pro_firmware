import kivy
kivy.require('1.8.0')

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.spinner import Spinner
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from utils import *
from autosportlabs.racecapture.views.configuration.rcp.baseconfigview import BaseConfigView
from rcpconfig import *
from mappedspinner import MappedSpinner

Builder.load_file('autosportlabs/racecapture/views/configuration/rcp/gpiochannelsview.kv')

class GPIOModeSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(GPIOModeSpinner, self).__init__(**kwargs)
        self.setValueMap({0:'Input', 1:'Output'}, 'Input')
        
class GPIOChannel(BoxLayout):
    channelConfig = None
    channels = None
    def __init__(self, **kwargs):
        super(GPIOChannel, self).__init__(**kwargs)
        kvFind(self, 'rcid', 'sr').bind(on_sample_rate = self.on_sample_rate)
        kvFind(self, 'rcid', 'chanId').bind(on_channel = self.on_channel)
        
    def on_channel(self, instance, value):
        if self.channelConfig:
            self.channelConfig.channelId = self.channels.getIdForName(value)

    def on_sample_rate(self, instance, value):
        if self.channelConfig:
            self.channelConfig.sampleRate = value
        
    def on_mode(self, instance, value):
        if self.channelConfig:
            self.channelConfig.mode = instance.getValueFromKey(value)
            
    def on_config_updated(self, channelConfig, channels):
        sampleRateSpinner = kvFind(self, 'rcid', 'sr')
        sampleRateSpinner.setValue(channelConfig.sampleRate)
    
        channelSpinner = kvFind(self, 'rcid', 'chanId')
        channelSpinner.setValue(channels.getNameForId(channelConfig.channelId))
        
        modeSpinner = kvFind(self, 'rcid', 'mode')
        modeSpinner.setFromValue(channelConfig.mode)
        
        self.channelConfig = channelConfig
        self.channels = channels
        
class GPIOChannelsView(BaseConfigView):
    editors = []
    def __init__(self, **kwargs):
        super(GPIOChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        self.channelCount = kwargs['channelCount']
        self.channels = kwargs['channels']
        
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=90 * self.channelCount)
    
        editors = []
        for i in range(self.channelCount):
            channel = AccordionItem(title='Digital Input/Output ' + str(i + 1))
            editor = GPIOChannel(id='gpio' + str(i))
            editors.append(editor)
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        self.editors = editors
        
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

    def on_config_updated(self, rcpCfg):
        gpioCfg = rcpCfg.gpioConfig
        channelCount = gpioCfg.channelCount

        for i in range(channelCount):
            editor = self.editors[i]
            gpioChannel = gpioCfg.channels[i]
            editor.on_config_updated(gpioChannel, self.channels)
            
            
            
            

            
 
