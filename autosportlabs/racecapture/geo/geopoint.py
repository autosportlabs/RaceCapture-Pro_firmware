
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
