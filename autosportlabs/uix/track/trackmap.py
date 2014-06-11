import kivy
import math
kivy.require('1.8.0')
from kivy.uix.label import Label
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.widget import Widget
from kivy.uix.scatter import Scatter
from kivy.graphics import Rectangle
from kivy.app import Builder
from kivy.metrics import dp
from kivy.graphics import Color, Line
from autosportlabs.racecapture.geo.geopoint import GeoPoint
from utils import *

Builder.load_file('autosportlabs/uix/track/trackmap.kv')

class Point:
    x = 0.0
    y = 0.0
    def __init__(self, x, y):
        self.x = x
        self.y = y
    
class TrackMap(Widget):
    MIN_PADDING = dp(1)
    offsetPoint = Point(0,0)
    globalRatio = 0
    heightPadding = 0
    widthPadding = 0
    
    def __init__(self, **kwargs):
        super(TrackMap, self).__init__(**kwargs)
        self.bind(pos=self.update_map)
        self.bind(size=self.update_map)

    def update_map(self, *args):
        pass
        
    def setTrackPoints(self, geoPoints):
        points = self.genMapPoints(geoPoints)
        linePoints = []
        for point in points:
            scaledPoint = self.scalePoint(point, self.height)
            linePoints.append(scaledPoint.x)
            linePoints.append(scaledPoint.y)
        
        with self.canvas.after:
            Color(1., 0, 0)
            Line(points=linePoints, width=dp(2), closed=True)
        
        
    def projectPoint(self, geoPoint):
        latitude = geoPoint.latitude * float(math.pi / 180.0)
        longitude = geoPoint.longitude * float(math.pi / 180.0)
        point = Point(longitude, float(math.log(math.tan((math.pi / 4.0) + 0.5 * latitude))))
        return point;

    def scalePoint(self, point, height):
        adjustedX = int((self.widthPadding + (point.x * self.globalRatio)))
        #need to invert the Y since 0,0 starts at top left
        adjustedY = int((self.heightPadding + (point.y * self.globalRatio)))
        return Point(adjustedX, adjustedY)

    def genMapPoints(self, geoPoints):
        points = []
        
        # min and max coordinates, used in the computation below
        minXY = Point(-1, -1)
        maxXY = Point(-1, -1)
        
        for geoPoint in geoPoints:
            point = self.projectPoint(geoPoint)
            minXY.x = point.x if minXY.x == -1 else min(minXY.x, point.x)
            minXY.y = point.y if minXY.y == -1 else min(minXY.y, point.y)
            points.append(point);
        
        # now, we need to keep track the max X and Y values
        for point in points:
            point.x = point.x - minXY.x
            point.y = point.y - minXY.y
            maxXY.x = point.x if maxXY.x == -1 else max(maxXY.x, point.x)
            maxXY.y = point.y if maxXY.y == -1 else max(maxXY.y, point.y);
        
            point.x += self.pos[0]
            point.y += self.pos[1]
        
        
        
        paddingBothSides = self.MIN_PADDING * 2
        
        width = float(self.width)
        height = float(self.height)

        print(str(width) + ' ' + str(height))        
        # the actual drawing space for the map on the image
        mapWidth = width - paddingBothSides;
        mapHeight = height - paddingBothSides;

        print(str(mapWidth) + ' ' + str(mapHeight))
        #determine the width and height ratio because we need to magnify the map to fit into the given image dimension
        mapWidthRatio = float(mapWidth) / float(maxXY.x)
        mapHeightRatio = float(mapHeight) / float(maxXY.y)

        # using different ratios for width and height will cause the map to be stretched. So, we have to determine
        # the global ratio that will perfectly fit into the given image dimension
        self.globalRatio = min(mapWidthRatio, mapHeightRatio);

        #now we need to readjust the padding to ensure the map is always drawn on the center of the given image dimension
        self.heightPadding = (height - (self.globalRatio * maxXY.y)) / 2.0
        self.widthPadding = (width - (self.globalRatio * maxXY.x)) / 2.0
        self.offsetPoint = minXY;
        
        return points
        
        
        
