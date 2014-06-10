import kivy
import math
kivy.require('1.8.0')
from kivy.uix.label import Label
from kivy.uix.widget import Widget
from kivy.app import Builder


class Point:
    x = 0.0
    y = 0.0
    def __init__(self, x, y):
        self.x = x
        self.y = y

class TrackMap(Widget):
    offsetPoint = Point()
    def __init__(self, **kwargs):
        super(TrackMap, self).__init__(**kwargs)
        
    def projectPoint(self, geoPoint):
        latitude = geoPoint.latitude * float(math.pi / 180.0)
        longitude = geoPoint.longitude * float(math.pi / 180.0)
        point = Point(longitude, float(math.log(math.tan((math.pi / 4.0) + 0.5 * latitude))))
        return point;


    
