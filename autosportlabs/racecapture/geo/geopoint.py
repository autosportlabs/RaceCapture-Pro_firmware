
class GeoPoint:
    def __init__(self, **kwargs):
        self.latitude = 0
        self.longitude = 0
    
    @classmethod
    def fromPoint(cls, latitude, longitude):
        g = GeoPoint()
        g.latitude = latitude
        g.longitude = longitude
        return g
    
    def fromJson(self, geoPointJson):
        try:
            self.latitude = geoPointJson[0]
            self.longitude = geoPointJson[1]
        except:
            print('Malformed GeoPoint: ' + str(geoPointJson))
    
    def toJson(self):
        return [self.latitude, self.longitude]


class Region:
    name = None
    points = None
    def __init__(self, **kwargs):
        self.points = []
        self.name = ''
    def fromJson(self, regionJson):
        self.name = regionJson.get('name', self.name)
        pointsNode = regionJson.get('points')
        if pointsNode:
            for pointNode in pointsNode:
                p = GeoPoint()
                p.fromJson(pointNode)
                self.points.append(p)
    
    def toJson(self):
        pass
    
    def withinRegion(self, geoPoint):
        inside = False
        if geoPoint:
            n = len(self.points)
            y = geoPoint.latitude
            x = geoPoint.longitude
            firstPoint = self.points[0]
            p1x = firstPoint[0]
            p1y = firstPoint[1]
            for i in range(n+1):
                pt = self.points[i % n]
                p2x = pt[0]
                p2y = pt[1]
                if y > min(p1y,p2y):
                    if y <= max(p1y,p2y):
                        if x <= max(p1x,p2x):
                            if p1y != p2y:
                                xints = (y-p1y)*(p2x-p1x)/(p2y-p1y)+p1x
                            if p1x == p2x or x <= xints:
                                inside = not inside
                p1x,p1y = p2x,p2y
        return inside        