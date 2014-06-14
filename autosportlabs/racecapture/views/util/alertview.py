import kivy
kivy.require('1.8.0')
from kivy.uix.popup import Popup
from kivy.uix.label import Label
from kivy.metrics import dp
__all__ = ('alertPopup')

def alertPopup(title, msg):
    popup = Popup(title = title,
                      content=Label(text = msg),
                      size_hint=(None, None), size=(dp(600), dp(200)))
    popup.open()    
    