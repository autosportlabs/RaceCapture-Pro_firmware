import kivy
kivy.require('1.8.0')

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.spinner import Spinner
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from utils import *
from autosportlabs.racecapture.views.configuration.baseconfigview import BaseConfigView
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
                    
    def on_mode(self, instance, value):
        if self.channelConfig:
            self.channelConfig.mode = instance.getValueFromKey(value)
            self.channelConfig.stale = True
            self.dispatch('on_modified', self.channelConfig)
            
    def on_config_updated(self, channelConfig, channels):
        self.channelConfig = channelConfig
        self.channels = channels

        sampleRateSpinner = kvFind(self, 'rcid', 'sr')
        sampleRateSpinner.setValue(channelConfig.sampleRate)
    
        channelSpinner = kvFind(self, 'rcid', 'chanId')
        channelSpinner.setValue(channels.getNameForId(channelConfig.channelId))
        
        modeSpinner = kvFind(self, 'rcid', 'mode')
        modeSpinner.setFromValue(channelConfig.mode)
        
        
class GPIOChannelsView(BaseConfigView):
    editors = []
    channels = None
    accordion = None
    gpioCfg = None
    def __init__(self, **kwargs):
        super(GPIOChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        self.channelCount = kwargs['channelCount']
        
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=90 * self.channelCount)
        self.accordion = accordion
        
        editors = []
        for i in range(self.channelCount):
            channel = AccordionItem(title='Digital Input/Output ' + str(i + 1))
            editor = GPIOChannel(id='gpio' + str(i))
            editors.append(editor)
            editor.bind(on_modified=self.on_modified)            
            channel.add_widget(editor)
            accordion.add_widget(channel)
        
        accordion.select(accordion.children[-1])
        self.editors = editors
        
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)


    def on_modified(self, instance, channelConfig):
        self.setAccordionItemTitle(self.accordion, self.gpioCfg.channels, channelConfig)
        super(GPIOChannelsView, self).on_modified(self, instance, channelConfig)

    def on_config_updated(self, rcpCfg):
        gpioCfg = rcpCfg.gpioConfig
        channels = rcpCfg.channels
        self.channels = channels 
        self.gpioCfg = gpioCfg

        gpioChannelCount = gpioCfg.channelCount
        for i in range(gpioChannelCount):
            editor = self.editors[i]
            gpioChannel = gpioCfg.channels[i]
            self.setAccordionItemTitle(self.accordion, gpioCfg.channels, gpioChannel)            
            editor.on_config_updated(gpioChannel, channels)
            
            
            
            

            
 
