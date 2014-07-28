import kivy
kivy.require('1.8.0')
from utils import *
from kivy.uix.stacklayout import StackLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.progressbar import ProgressBar
from kivy.app import Builder
from kivy.clock import Clock
from iconbutton import IconButton

Builder.load_file('toolbarview.kv')

TOOLBAR_LED_DURATION = 0.1
PROGRESS_COMPLETE_LINGER_DURATION = 5.0

class ToolbarView(BoxLayout):
    txOffColor = [0.0, 1.0, 0.0, 0.2]
    rxOffColor = [0.0, 0.8, 1.0, 0.2]
    txOnColor = [0.0, 1.0, 0.0, 1.0]
    rxOnColor = [0.0, 0.8, 1.0, 1.0]

    normalStatusColor = [0.8, 0.8, 0.8, 1.0]
    alertStatusColor = [1.0, 0.64, 0.0, 1.0]
    
    progressBar = None
    teleTxStatus = None
    teleRxStatus = None
    rcTxStatus = None
    rcRxStatus = None
    def __init__(self, **kwargs):
        super(ToolbarView, self).__init__(**kwargs)
        self.register_event_type('on_main_menu')
        self.register_event_type('on_progress')
        self.register_event_type('on_rc_tx')
        self.register_event_type('on_rc_rx')
        self.register_event_type('on_tele_tx')
        self.register_event_type('on_tele_rx')
        self.register_event_type('on_status')
                    
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
    def on_status(self, msg, isAlert):
        statusLabel = kvFind(self, 'rcid', 'status')
        statusLabel.text = msg
        if isAlert == True:
            statusLabel.color = self.alertStatusColor
        else:
            statusLabel.color = self.normalStatusColor
            
        
    def on_progress(self, value):
        if not self.progressBar:
            self.progressBar = kvFind(self, 'rcid', 'pbar')
        self.progressBar.value = value
        if value == 100:
            Clock.schedule_once(lambda dt: self.on_progress(0), PROGRESS_COMPLETE_LINGER_DURATION)
        
    def on_main_menu(self, instance, *args):
        pass
        
    def mainMenu(self):
        self.dispatch('on_main_menu', None)
    
    def on_rc_tx_decay(self, dt):
        self.on_rc_tx(False)
        
    def on_rc_tx(self, value):
        if not self.rcTxStatus:
            self.rcTxStatus = kvFind(self, 'rcid', 'rcTxStatus')            
        self.rcTxStatus.color = self.txOnColor if value else self.txOffColor
        Clock.unschedule(self.on_rc_tx_decay)
        Clock.schedule_once(self.on_rc_tx_decay, TOOLBAR_LED_DURATION)
    
    def on_rc_rx_decay(self, dt):
        self.on_rc_rx(False)
        
    def on_rc_rx(self, value):
        if not self.rcRxStatus:
            self.rcRxStatus = kvFind(self, 'rcid', 'rcRxStatus')    
        self.rcRxStatus.color = self.rxOnColor if value else self.rxOffColor
        Clock.unschedule(self.on_rc_rx_decay)
        Clock.schedule_once(self.on_rc_rx_decay, TOOLBAR_LED_DURATION)
    
    def on_tele_tx_decay(self):
        self.on_tele_tx(False)
        
    def on_tele_tx(self, value):
        if not self.teleTxStatus:
            self.teleTxStatus = kvFind(self, 'rcid', 'teleRxStatus')
        self.teleTxStatus.color = self.txOnColor if value else self.txOnColor
        Clock.unschedule(self.on_tele_tx_decay)
        Clock.schedule_once(self.on_tele_tx_decay, TOOLBAR_LED_DURATION)

    def on_tele_rx_decay(self):
        self.on_tele_rx(False)
        
    def on_tele_rx(self, value):
        if not self.teleRxStatus:
            self.teleRxStatus = kvFind(self, 'rcid', 'teleTxStatus')
        self.teleRxStatus.color = self.rxOnColor if value else self.rxOffColor
        Clock.unschedule(self.on_tele_rx_decay)
        Clock.schedule_once(self.on_tele_rx_decay, TOOLBAR_LED_DURATION)
    
