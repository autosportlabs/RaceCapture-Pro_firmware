import json
import time
import copy
from urlparse import urljoin, urlparse
import urllib2
from autosportlabs.racecapture.geo.geopoint import GeoPoint

from autosportlabs.racecapture.geo.geopoint import GeoPoint

class Venue:
    venueId = None
    uri = None
    updatedAt = None
    def __init__(self, **kwargs):
        self.venueId = str(kwargs.get('venueId', self.venueId))
        self.uri = str(kwargs.get('uri', self.uri))
        self.updatedAt = str(kwargs.get('updatedAt', self.updatedAt))
        
    def fromJson(self, venueJson):
        self.venueId = venueJson.get('id', self.venueId)
        self.uri = venueJson.get('URI', self.uri)
        self.updatedAt = venueJson.get('updated', self.updatedAt)
        
class TrackMap:
    mapPoints = None
    sectorPoints = None
    name = ''
    updatedAt = None
    length = 0
    def __init__(self, **kwargs):
        pass
    
    def fromJson(self, trackJson):
        venueNode = trackJson.get('venue')
        if (venueNode):
            self.updatedAt = venueNode.get('updated', self.updatedAt)
            self.name = venueNode.get('name', self.name)
            self.length = venueNode.get('length', self.length)
            mapPointsNode = venueNode.get('track_map_array')
            mapPoints = []
            if mapPointsNode:
                for point in mapPointsNode:
                    mapPoints.append(GeoPoint.fromPoint(point[0], point[1]))
            self.mapPoints = mapPoints
                    
            sectorNode = venueNode.get('sector_points')
            sectorPoints = []
            if sectorNode:
                for point in sectorNode:
                    self.sectorPoints.append(GeoPoint.fromPoint(point[0], point[1]))
            self.sectorPoints = sectorPoints
        
class TrackManager:
    user_dir = '.'
    track_user_subdir = '/venues'
    on_progress = lambda self, value: value
    rcp_venue_url = 'http://race-capture.com/api/v1/venues'
    readRetries = 3
    retryDelay = 1.0
    trackList = {}
    tracks = {}
    def __init__(self, **kwargs):
        self.user_dir = kwargs.get('user_dir', self.user_dir) + self.track_user_subdir
        
    def load_tracks(self):
        pass
    
    def findClosestTrack(self, point, radius):
        pass

    def loadJson(self, uri):
        retries = 0
        while retries < self.readRetries:
            try:
                opener = urllib2.build_opener()
                opener.addheaders = [('Accept', 'application/json')]
                jsonStr = opener.open(uri).read()
                j = json.loads(jsonStr)
                return j
            except Exception as detail:
                print('Failed to read: ' + str(detail))
                if retries < self.readRetries:
                    print('retrying in ' + str(self.retryDelay) + ' seconds...')
                    retries += 1
                    time.sleep(self.retryDelay)
            raise Exception('Error reading json doc from: ' + uri)    
                
    def downloadTrackList(self):
        start = 0
        totalVenues = None
        nextUri = self.rcp_venue_url + '?start=' + str(start)
        self.trackList.clear()
        while nextUri:
            venuesDocJson = self.loadJson(nextUri)
            try:
                if totalVenues == None:
                    totalVenues = int(venuesDocJson.get('total', None))
                    if totalVenues == None:
                        raise Exception('Malformed venue list JSON: could not get total venue count')
                venuesListJson = venuesDocJson.get('venues', None)
                if venuesListJson == None:
                    raise Exception('Malformed venue list JSON: could not get venue list')
                for venueJson in venuesListJson:
                    venue = Venue()
                    venue.fromJson(venueJson)
                    self.trackList[venue.venueId] = venue
                    print('found venue id: ' + venue.venueId)
                                
                nextUri = venuesDocJson.get('nextURI')
            except Exception as detail:
                print('Malformed venue JSON from url ' + nextUri + '; json =  ' + str(venueJson) + ' ' + str(detail))
                
        retrievedVenueCount = len(self.trackList)                 
        if (not totalVenues == retrievedVenueCount):
            print('Warning - reported track count not reflect number of tracks retrieved: ' + str(totalVenues) + '/' + str(retrievedVenueCount))
        
    def downloadTrack(self, venueId):
        trackUrl = self.rcp_venue_url + '/' + venueId
        trackJson = self.loadJson(trackUrl)
        trackMap = TrackMap()
        trackMap.fromJson(trackJson)
        
        return copy.deepcopy(trackMap)
        
    def downloadAllTracks(self):
        self.downloadTrackList()
        self.tracks.clear()
        for trackId in self.trackList.keys():
            print('downloading track: ' + trackId)
            trackMap = self.downloadTrack(trackId)
            self.tracks[trackId] = trackMap
            
            
            
            
        
        
        
        
    
    
    