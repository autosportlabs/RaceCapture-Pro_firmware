import kivy
kivy.require('1.8.0')
from kivy.uix.button import Button
from kivy.app import Builder

Builder.load_file('iconbutton.kv')

class IconButton(Button):
    pass
