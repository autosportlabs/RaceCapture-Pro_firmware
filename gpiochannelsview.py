import kivy
kivy.require('1.8.0')

from kivy.uix.boxlayout import BoxLayout
from kivy.uix.spinner import Spinner
from kivy.uix.accordion import Accordion, AccordionItem
from kivy.uix.scrollview import ScrollView
from kivy.app import Builder
from utils import *

from rcpconfig import *
from mappedspinner import MappedSpinner

Builder.load_file('gpiochannelsview.kv')



class GPIOModeSpinner(MappedSpinner):
    def __init__(self, **kwargs):
        super(GPIOModeSpinner, self).__init__(**kwargs)
        self.setValueMap({0:'Input', 1:'Output'}, 'Input')
        
class GPIOChannel(BoxLayout):
    def __init__(self, **kwargs):
        super(GPIOChannel, self).__init__(**kwargs)

class GPIOChannelsView(BoxLayout):
    def __init__(self, **kwargs):
        super(GPIOChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')
        self.channelCount = kwargs['channelCount']
        self.channels = kwargs['channels']
        
        accordion = Accordion(orientation='vertical', size_hint=(1.0, None), height=90 * self.channelCount)
    
        for i in range(self.channelCount):
            channel = AccordionItem(title='Digital Input/Output ' + str(i + 1))
            editor = GPIOChannel(id='gpio' + str(i))
            channel.add_widget(editor)
            accordion.add_widget(channel)
    
        sv = ScrollView(size_hint=(1.0,1.0), do_scroll_x=False)
        sv.add_widget(accordion)
        self.add_widget(sv)

    def on_config_updated(self, rcpCfg):
        gpioCfg = rcpCfg.gpioConfig
        channelCount = gpioCfg.channelCount

        for i in range(channelCount):
            
            editor = kvFind(self, 'id', 'gpio' + str(i))
            gpioChannel = gpioCfg.channels[i]
            
            channelSpinner = kvFind(editor, 'rcid', 'chan')
            channelSpinner.setValue(self.channels.getNameForId(gpioChannel.channelId))

            sampleRateSpinner = kvFind(editor, 'rcid', 'sr')
            sampleRateSpinner.setValue(gpioChannel.sampleRate)
            
            modeSpinner = kvFind(editor, 'rcid', 'mode')
            modeSpinner.setFromValue(gpioChannel.mode)
            
            
            
            

            
 
