import kivy
kivy.require('1.8.0')
from kivy.metrics import dp
from kivy.uix.popup import Popup
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from kivy.uix.label import Label
from kivy.uix.accordion import Accordion, AccordionItem
from autosportlabs.widgets.separator import HSeparator, HSeparatorMinor
from utils import *
from rcpconfig import *
from channels import Channels, Channel

Builder.load_file('autosportlabs/racecapture/views/channels/channelsview.kv')

class ChannelLabel(Label):
    def __init__(self, **kwargs):
        super(ChannelLabel, self).__init__(**kwargs)
    
class ChannelView(BoxLayout):
    channel = None
    name = None
    def __init__(self, **kwargs):
        super(ChannelView, self).__init__(**kwargs)
        self.channel = kwargs.get('channel', None)
        self.name = kwargs.get('name', '')
        kvFind(self, 'rcid', 'sysChan').text = '\357\200\243' if self.channel.systemChannel else ''
        deleteButton = kvFind(self, 'rcid', 'delete')
        deleteButton.disabled = self.channel.systemChannel
        deleteButton.text = '\357\200\224' 
        kvFind(self, 'rcid', 'name').text = self.channel.name + ('' if self.channel.units == '' else ' (' + self.channel.units + ')')
        
    def on_edit(self):
        channel = self.channel
        popup = Popup(title = 'Edit System Channel' if channel.systemChannel else 'Edit Channel',
                      content = ChannelEditor(channel = channel), 
                      size_hint=(None, None), size = (dp(500), dp(180)))
        popup.open()

class ChannelEditor(BoxLayout):
    channel = None
    def __init__(self, **kwargs):
        super(ChannelEditor, self).__init__(**kwargs)
        self.channel = kwargs.get('channel', None)
        self.initView()
        
    def initView(self):
        nameField = kvFind(self, 'rcid', 'name')
        unitsField = kvFind(self, 'rcid', 'units')
        precisionField = kvFind(self, 'rcid', 'prec')
        minField = kvFind(self, 'rcid', 'min')
        maxField = kvFind(self, 'rcid', 'max')
        
        nameField.set_next(unitsField)
        unitsField.set_next(precisionField)
        precisionField.set_next(minField)
        minField.set_next(maxField)
        maxField.set_next(nameField)
        
        if self.channel:
            channel = self.channel
            nameField.text = channel.name
            nameField.disabled = channel.systemChannel
            unitsField.text = channel.units
            precisionField.text = str(channel.precision)
            minField.text = str(channel.min)
            maxField.text = str(channel.max)
            
    def on_name(self, instance, value):
        self.channel.name = value
        
    def on_units(self, instance, value):
        self.channel.units = value
    
    def on_precision(self, instance, value):
        self.channel.precision = int(value)
    
    def on_min(self, instance, value):
        self.channel.min = float(value)
    
    def on_max(self, instance, value):
        self.channel.max = float(value)
        

class ChannelsView(BoxLayout):
    channelsContainer = None
    channels = None
    def __init__(self, **kwargs):
        super(ChannelsView, self).__init__(**kwargs)
        self.register_event_type('on_channels_updated')
        self.channelsContainer = kvFind(self, 'rcid', 'channelsContainer')
     
    def on_channels_updated(self, channels):
        self.channelsContainer.clear_widgets()
        for channel in channels.items:
            self.channelsContainer.add_widget(ChannelView(channel=channel))
        kvFind(self, 'rcid', 'addChan').disabled = False
            
