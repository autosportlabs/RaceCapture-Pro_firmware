import kivy
kivy.require('1.8.0')
from kivy.uix.button import Button
from kivy.uix.boxlayout import BoxLayout
from kivy.app import Builder
from kivy.graphics import Color
from kivy.properties import NumericProperty, ListProperty, StringProperty, ObjectProperty
from math import sin, cos, pi

Builder.load_file('iconbutton.kv')

class IconButton(Button):
    pass
    
    
class TileIconButton(BoxLayout):
    line_color = ObjectProperty((0.5, 0.5, 0.5, 0.8))
    icon_color = ObjectProperty((1.0, 1.0, 1.0, 0.8))
    title_color = ObjectProperty((1.0, 1.0, 1.0, 0.8))
    icon = StringProperty('')
    title = StringProperty('')
    corners = ListProperty([0, 0, 0, 0])
    line_width = NumericProperty(1)
    resolution = NumericProperty(100)
    points = ListProperty([])
 
    def __init__(self, **kwargs):
        super(TileIconButton, self).__init__(**kwargs)
        self.register_event_type('on_press')

    def on_button_press(self, *args):
        self.dispatch('on_press')
        
    def on_press(self, *args):
        pass
    
    def compute_points(self, *args):
        self.points = []
 
        a = - pi
 
        x = self.x + self.corners[0]
        y = self.y + self.corners[0]
        while a < - pi / 2.:
            a += pi / self.resolution
            self.points.extend([
                x + cos(a) * self.corners[0],
                y + sin(a) * self.corners[0]
                ])
 
        x = self.right - self.corners[1]
        y = self.y + self.corners[1]
        while a < 0:
            a += pi / self.resolution
            self.points.extend([
                x + cos(a) * self.corners[1],
                y + sin(a) * self.corners[1]
                ])
 
        x = self.right - self.corners[2]
        y = self.top - self.corners[2]
        while a < pi / 2.:
            a += pi / self.resolution
            self.points.extend([
                x + cos(a) * self.corners[2],
                y + sin(a) * self.corners[2]
                ])
 
        x = self.x + self.corners[3]
        y = self.top - self.corners[3]
        while a < pi:
            a += pi / self.resolution
            self.points.extend([
                x + cos(a) * self.corners[3],
                y + sin(a) * self.corners[3]
                ])
 
        self.points.extend(self.points[:2])
 
