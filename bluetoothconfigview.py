import kivy
kivy.require('1.8.0')
from kivy.app import Builder
from kivy.uix.boxlayout import BoxLayout

from settingsview import SettingsView
from separator import HLineSeparator
from valuefield import ValueField

Builder.load_file('bluetoothconfigview.kv')

class BluetoothTelemetryView(BoxLayout):
    def __init__(self, **kwargs):
        super(BluetoothTelemetryView, self).__init__(**kwargs)
        self.register_event_type('on_config_updated')

    def on_config_updated(self, rcpCfg):
        pass
