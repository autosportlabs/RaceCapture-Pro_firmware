import kivy
kivy.require('1.8.0')
from kivy.uix.button import Button
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.anchorlayout import AnchorLayout
from kivy.app import Builder
from kivy.graphics import Color
from kivy.properties import NumericProperty, ListProperty, StringProperty, ObjectProperty
from math import sin, cos, pi

Builder.load_file('iconbutton.kv')

class IconButton(Button):
    def __init__(self, **kwargs):
        self.font_name = 'resource/fonts/fa.ttf'
        super(IconButton, self).__init__(**kwargs)

class RoundedRect(BoxLayout):
    rect_color = ObjectProperty((0.5, 0.5, 0.5, 0.8))
    corners = ListProperty([0, 0, 0, 0])
    line_width = NumericProperty(1)
    resolution = NumericProperty(100)
    points = ListProperty([])
    
    def compute_points(self, *args):
        self.points = []
 
        startX = self.x
        startY = self.y
        startRight = self.right
        startTop = self.top
        
        lineWidth = self.line_width        
        while startRight > startX - lineWidth:
            a = - pi
     
            x = startX + self.corners[0]
            y = startY + self.corners[0]
            while a < - pi / 2.:
                a += pi / self.resolution
                self.points.extend([
                    x + cos(a) * self.corners[0],
                    y + sin(a) * self.corners[0]
                    ])
     
            x = startRight - self.corners[1]
            y = startY + self.corners[1]
            while a < 0:
                a += pi / self.resolution
                self.points.extend([
                    x + cos(a) * self.corners[1],
                    y + sin(a) * self.corners[1]
                    ])
     
            x = startRight - self.corners[2]
            y = startTop - self.corners[2]
            while a < pi / 2.:
                a += pi / self.resolution
                self.points.extend([
                    x + cos(a) * self.corners[2],
                    y + sin(a) * self.corners[2]
                    ])
     
            x = startX + self.corners[3]
            y = startTop - self.corners[3]
            while a < pi:
                a += pi / self.resolution
                self.points.extend([
                    x + cos(a) * self.corners[3],
                    y + sin(a) * self.corners[3]
                    ])
     
            self.points.extend(self.points[:2])
            
            startX += lineWidth
            startRight -= lineWidth
            startTop -= lineWidth
            startY += lineWidth
        
class TileIconButton(AnchorLayout):
    title_font = StringProperty('')
    title_font_size = NumericProperty(20)
    tile_color = ObjectProperty((0.5, 0.5, 0.5, 0.8))    
    icon_color = ObjectProperty((1.0, 1.0, 1.0, 0.8))
    title_color = ObjectProperty((1.0, 1.0, 1.0, 0.8))
    icon = StringProperty('')
    title = StringProperty('')
 
    def __init__(self, **kwargs):
        super(TileIconButton, self).__init__(**kwargs)
        self.register_event_type('on_press')

    def on_button_press(self, *args):
        self.dispatch('on_press')
        
    def on_press(self, *args):
        pass
    
 
