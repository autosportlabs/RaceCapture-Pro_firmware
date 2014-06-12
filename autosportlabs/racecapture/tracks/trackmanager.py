import json
import time
from urllib import urlopen

from autosportlabs.racecapture.geo.geopoint import GeoPoint

class Venue:
    venueId = None
    uri = None
    updatedAt = None
    def __init__(self, *kwargs):
        self.venueId = kwargs.get('venueId', self.venueId)
        self.uri = kwargs.get('uri', self.uri)
        self.updatedAt = kwargs.get('updatedAt', self.updatedAt)
        
    def fromJson(self, venueJson):
        self.venueId = venueJson.get('id', self.venueId)
        self.uri = venueJson.get('URI', self.uri)
        self.updatedAt = venueJson.get('updated', self.updatedAt)
        
class TrackManager:
    user_dir = '.'
    track_user_subdir = '/venues'
    on_progress = lambda self, value: value
    rcp_venue_url = 'http://race-capture.com/api/v1/venues.json'
    readRetries = 3
    retryDelay = 1.0
    trackList = {}
    tracks = {}
    def __init__(self, **kwargs):
        super(TrackManager, self).__init__(**kwargs)
        self.user_dir = kwargs.get('user_dir', self.user_dir) + self.track_user_subdir
        
    def load_tracks(self):
        pass
    
    def findClosestTrack(self, point, radius):
        pass

    def loadJson(self, uri):
        retries = 0
        while retries < self.readRetries:
            try:
                return json.loads(urlopen(uri).read())
            except:
                print('Failed to read')
                if retries < self.readRetries:
                    print('retrying in ' + self.retryDelay + ' seconds...')
                    retries += 1
                    time.sleep(self.retryDelay)
            raise 'Error reading json doc from: ' + uri    
                
    def downloadTrackList(self):
        
        start = 0
        totalVenues = None
        nextUri = self.rcp_venue_url + '?start=' + str(start)
        while nextUri:
            venuesDocJson = self.loadJson(self, nextUri)
            try:
                if totalVenues == None:
                    totalVenues = int(venuesDocJson.get('total', None))
                    if totalVenues == None:
                        raise 'Malformed venue list JSON: could not get total venue count'
                venuesListJson = venuesDocJson.get('venues', None)
                if venuesListJson == None:
                    raise 'Malformed venue list JSON: could not get venue list'
                for venueJson in venuesListJson:
                    venue = Venue()
                    venue.fromJson(venueJson)
                    self.trackList[venue.venueId] = venue
                    print('found venue id: ' + venue.venueId)
                nextUri = venuesDocJson.get('nextURI')
            except:
                print('Malformed venue JSON from url ' + nextUri + '; json =  ' + str(venueJson))
                
        retrievedVenueCount = len(self.trackList)                 
        if (not totalVenues == retrievedVenueCount):
            print('Warning - track list count does not reflect number of tracks retrieved: ' + str(totalVenues) + '/' + str(retrievedVenueCount))
        
                
            
            
            
        
        
        
        
    
    
    