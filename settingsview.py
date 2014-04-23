import kivy
kivy.require('1.8.0')

from kivy.uix.label import Label
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.switch import Switch
from fieldlabel import FieldLabel
from helplabel import HelpLabel
from kivy.app import Builder
from utils import *
from kivy.properties import StringProperty

Builder.load_file('settingsview.kv')

class SettingsView(BoxLayout):
    help_text = StringProperty('foobar')
    def __init__(self, **kwargs):
        super(SettingsView, self).__init__(**kwargs)
        fieldLabelText = kwargs.get('label_text', '')
        print("help texddddddddddddddddddddddddddt: " + self.help_text)        
        settingsCtrl = kwargs.get('settings_ctrl', '')
        
        fieldLabel = kvFind(self, 'rcid', 'fieldLabel')
        settingsCtrlContainer = kvFind(self, 'rcid', 'settingsCtrl')
        helpLabel = kvFind(self, 'rcid', 'helpLabel')
        
        fieldLabel.text = fieldLabelText
        helpLabel.text = helpText
        if settingsCtrl:
            settingsCtrlContainer.add_widget(settingsCtrl)
        
        
class SettingsViewSwitch(SettingsView):
    def __init__(self, **kwargs):
        switch = Switch()
        kwargs['settings_ctrl'] =  switch
        self.switch = switch
        print("the argsss" + str(kwargs))
        super(SettingsViewSwitch, self).__init__(**kwargs)
        
    
        
        

