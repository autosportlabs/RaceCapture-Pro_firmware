import json
import time
import copy
import errno
from threading import Thread, Lock
from os import listdir, makedirs
from urlparse import urljoin, urlparse
import urllib2
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
    trackId = None
    def __init__(self, **kwargs):
        pass
                        
    def fromJson(self, trackJson):
        venueNode = trackJson.get('venue')
        if (venueNode):
            self.updatedAt = venueNode.get('updated', self.updatedAt)
            self.name = venueNode.get('name', self.name)
            self.length = venueNode.get('length', self.length)
            self.trackId = venueNode.get('id', self.trackId)
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
    
    def toJson(self):
        venueJson = {}
        venueJson['updated'] = self.updatedAt
        venueJson['name'] = self.name
        venueJson['length'] = self.length
        venueJson['id'] = self.trackId
        trackPoints = []
        for point in self.mapPoints:
            trackPoints.append([point.latitude, point.longitude])
        venueJson['track_map_array'] = trackPoints
        
        sectorPoints = []
        for point in self.sectorPoints:
            sectorPoints.append(point.latitude, point.longitude)
        venueJson['sector_points'] = sectorPoints
        
        return {'venue': venueJson}
        
class TrackManager:
    updateLock = None
    tracks_user_dir = '.'
    track_user_subdir = '/venues'
    on_progress = lambda self, value: value
    rcp_venue_url = 'http://race-capture.com/api/v1/venues'
    readRetries = 3
    retryDelay = 1.0
    trackList = {}
    tracks = {}
    def __init__(self, **kwargs):
        self.setTracksUserDir(kwargs.get('user_dir', self.tracks_user_dir) + self.track_user_subdir)
        self.updateLock = Lock()
        
    def setTracksUserDir(self, path):
        try:
            print('here ' + path)
            makedirs(path)
            print("the path " + path)     
        except OSError as exception:
            if exception.errno != errno.EEXIST:
                raise
        self.tracks_user_dir = path
        
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
        trackList = {}
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
                    trackList[venue.venueId] = venue
                                
                nextUri = venuesDocJson.get('nextURI')
            except Exception as detail:
                print('Malformed venue JSON from url ' + nextUri + '; json =  ' + str(venueJson) + ' ' + str(detail))
                
        retrievedVenueCount = len(trackList)
        print('fetched list of ' + str(retrievedVenueCount) + ' tracks')                 
        if (not totalVenues == retrievedVenueCount):
            print('Warning - track list count does not reflect downloaded track list size ' + str(totalVenues) + '/' + str(retrievedVenueCount))
        return trackList
        
    def downloadTrack(self, venueId):
        trackUrl = self.rcp_venue_url + '/' + venueId
        trackJson = self.loadJson(trackUrl)
        trackMap = TrackMap()
        trackMap.fromJson(trackJson)
        
        return copy.deepcopy(trackMap)
        
    def saveTrack(self, trackMap, trackId):
        path = self.tracks_user_dir + '/' + trackId + '.json'
        trackJsonString = json.dumps(trackMap.toJson(), sort_keys=True, indent=2, separators=(',', ': '))
        with open(path, 'w') as text_file:
            text_file.write(trackJsonString)
    
    def loadCurrentTracksWorker(self, winCallback, failCallback, progressCallback=None):
        try:
            self.updateLock.acquire()
            self.loadCurrentTracks(progressCallback)
            winCallback()
        except Exception as detail:
            failCallback(detail)
        finally:
            self.updateLock.release()
        
    def loadCurrentTracks(self, progressCallback=None, winCallback=None, failCallback=None):
        if winCallback and failCallback:
            t = Thread(target=self.loadCurrentTracksWorker, args=(winCallback, failCallback, progressCallback))
            t.daemon=True
            t.start()
        else:
            existingTracksFilenames = listdir(self.tracks_user_dir)
            self.tracks.clear()
            self.trackList.clear()
            trackCount = len(existingTracksFilenames)
            count = 0
            for trackPath in existingTracksFilenames:
                try:
                    json_data = open(self.tracks_user_dir + '/' + trackPath)
                    trackJson = json.load(json_data)
                    trackMap = TrackMap()
                    trackMap.fromJson(trackJson)
                    
                    venueNode = trackJson['venue']
                    if venueNode:
                        venue = Venue()
                        venue.fromJson(venueNode)
                        self.tracks[venue.venueId] = trackMap
                    count += 1
                    if progressCallback:
                        progressCallback(count, trackCount, trackMap.name)
                except Exception as detail:
                    print('failed to read track file ' + trackPath + '; ' + str(detail))
                        
    def updateAllTracksWorker(self, winCallback, failCallback, progressCallback=None):
        try:
            self.updateLock.acquire()
            self.updateAllTracks(progressCallback)
            winCallback()
        except Exception as detail:
            failCallback(detail)
        finally:
            self.updateLock.release()
            
    def updateAllTracks(self, progressCallback=None, winCallback=None, failCallback=None):
        if winCallback and failCallback:
            t = Thread(target=self.updateAllTracksWorker, args=(winCallback, failCallback, progressCallback))
            t.daemon=True
            t.start()
        else:
            self.loadCurrentTracks()
            updatedTrackList = self.downloadTrackList()
            
            currentTracks = self.tracks
            updatedIds = updatedTrackList.keys()
            updatedCount = len(updatedIds)
            count = 0
            for trackId in updatedIds:
                updateTrack = False
                count += 1
                if currentTracks.get(trackId) == None:
                    print('new track detected ' + trackId)
                    updateTrack = True
                elif not currentTracks[trackId].updatedAt == updatedTrackList[trackId].updatedAt:
                    print('existing map changed ' + trackId)
                    updateTrack = True
                if updateTrack:
                    updatedTrackMap = self.downloadTrack(trackId)
                    self.saveTrack(updatedTrackMap, trackId)
                    if progressCallback:
                        progressCallback(count, updatedCount, updatedTrackMap.name)
                else:
                    progressCallback(count, updatedCount)
            self.loadCurrentTracks()
                
        
        
                    
            
        
            
        
        
            
            
        
        
        
        
    
    
    