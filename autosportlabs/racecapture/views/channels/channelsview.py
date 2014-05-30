import kivy
from rcpserial import CHANNEL_ADD_MODE_IN_PROGRESS, CHANNEL_ADD_MODE_COMPLETE
kivy.require('1.8.0')
from kivy.metrics import dp
from kivy.uix.popup import Popup
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.stacklayout import StackLayout
from kivy.app import Builder
from kivy.uix.label import Label
from kivy.uix.accordion import Accordion, AccordionItem
from autosportlabs.widgets.separator import HSeparator, HSeparatorMinor
from autosportlabs.racecapture.views.util.alertview import alertPopup
from utils import *
from rcpconfig import *
from channels import Channels, Channel

Builder.load_file('autosportlabs/racecapture/views/channels/channelsview.kv')

class ChannelLabel(Label):
    def __init__(self, **kwargs):
        super(ChannelLabel, self).__init__(**kwargs)
    
class ChannelView(BoxLayout):
    channel = None
    def __init__(self, **kwargs):
        super(ChannelView, self).__init__(**kwargs)
        self.channel = kwargs.get('channel', self.channel)
        self.register_event_type('on_delete_channel')
        self.updateView()
        
    def updateView(self):
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
        popup.bind(on_dismiss=self.on_edited)
        
    def on_delete(self):
        self.dispatch('on_delete_channel', self.channel)
        
    def on_delete_channel(self, *args):
        pass
        
    def on_edited(self, *args):
        self.updateView()

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
    rcpComms = None
    def __init__(self, **kwargs):
        super(ChannelsView, self).__init__(**kwargs)
        self.channels = kwargs.get('channels', self.channels)
        self.rcpComms = kwargs.get('rcpComms', self.rcpComms)
        self.register_event_type('on_channels_updated')
        self.channelsContainer = kvFind(self, 'rcid', 'channelsContainer')
     
    def on_channels_updated(self, channels):
        self.channelsContainer.clear_widgets()
        for channel in channels.items:
            channelView = ChannelView(channel=channel)
            channelView.bind(on_delete_channel = self.on_delete_channel)
            self.channelsContainer.add_widget(channelView)
        self.channels = channels
        kvFind(self, 'rcid', 'addChan').disabled = False
            
    def on_delete_channel(self, instance, value):
        channelItems = self.channels.items
        for channel in channelItems:
            if value == channel:
                channelItems.remove(channel)
                break
        self.on_channels_updated(self.channels)
        
    def on_add_channel(self):
        newChannel = Channel(name='Channel', units='',precision=0, min=0, max=100)
        self.channels.items.append(newChannel)
        channelView = ChannelView(channel=newChannel)
        channelView.bind(on_delete_channel = self.on_delete_channel)
        self.channelsContainer.add_widget(channelView)
        channelView.on_edit()
        
        
        try:
            with open(os.path.join(path, filename[0])) as stream:
                self.rcpConfig.fromJsonString(stream.read())
        except Exception as detail:
            alertPopup('Error Loading', 'Failed to Load Configuration:\n\n' + str(detail))
        
    def on_read_channels(self):
        if self.rcpComms:
            try:
                channelsJson = self.rcpComms.getChannels()
                self.channels.fromJson(channelsJson)
                self.on_channels_updated(self.channels)
            except Exception as detail:
                alertPopup('Error Reading', 'Could not read channels:\n\n' + str(detail))
        
    def on_write_channels(self):
        if self.rcpComms:
            try:
                index = 0
                channelCount = len(self.channels.items)
                for channel in self.channels.items:
                    mode = CHANNEL_ADD_MODE_IN_PROGRESS if index < channelCount - 1 else CHANNEL_ADD_MODE_COMPLETE
                    self.rcpComms.addChannel( channel.toJson(), index, mode)
                    index += 1
            except Exception as detail:
                alertPopup('Error Writing', 'Could not write channels:\n\n' + str(detail))
        else:
            alertPopup('Warning', 'Please load channels before writing')
            
