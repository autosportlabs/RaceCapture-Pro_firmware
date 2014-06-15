import json
import time
import copy
import errno
import string
import logging
from threading import Thread, Lock
from os import listdir, makedirs
from urlparse import urljoin, urlparse
import urllib2
from autosportlabs.racecapture.geo.geopoint import GeoPoint, Region
        
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
        self.mapPoints = []
        self.sectorPoints = []
        pass
                    
    def getCenterPoint(self):
        if len(self.mapPoints) > 0:
            return self.mapPoints[0]
        return None
    
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
    trackList = None
    tracks = None
    regions = None
    regionTrackIds = None
    def __init__(self, **kwargs):
        self.setTracksUserDir(kwargs.get('user_dir', self.tracks_user_dir) + self.track_user_subdir)
        self.updateLock = Lock()
        self.regions = {}
        self.trackList = {}
        self.tracks = {}
        self.regionTrackIds = []
        
    def setTracksUserDir(self, path):
        try:
            makedirs(path)
        except OSError as exception:
            if exception.errno != errno.EEXIST:
                raise
        self.tracks_user_dir = path
        
    def init(self, progressCallback, winCallback, failCallback):
        self.loadRegions()
        self.loadCurrentTracks(progressCallback, winCallback, failCallback)
        
    def loadRegions(self):
        self.regions.clear()
        try:
            regionsJson = json.load(open('resource/settings/geo_regions.json'))
            regionsNode = regionsJson.get('regions')
            if regionsNode:
                for regionNode in regionsNode: 
                    region = Region()
                    region.fromJson(regionNode)
                    self.regions[region.name] = region
        except Exception as detail:
            print('Error loading regions data ' + str(detail))
    
    def getAllTrackIds(self):
        return self.tracks.keys()
    
    def getRegionTrackIds(self):
        return self.regionTrackIds
        
    def filterTracksByName(self, name, trackIds=None):
        if trackIds == None:
            trackIds = self.tracks.keys()
        filteredTrackIds = []
        for trackId in trackIds:
            trackName = self.tracks[trackId].name
            if string.lower(name.strip()) in string.lower(trackName.strip()):
                filteredTrackIds.append(trackId)
        return filteredTrackIds
                
    def filterTracksByRegion(self, regionName):
        allTrackIds = self.tracks.keys()
        regionTrackIds = self.regionTrackIds
        del regionTrackIds[:]
        
        if regionName == None:
            regionTrackIds.extend(allTrackIds)
        else:
            for name in self.regions.keys():
                if name == regionName:
                    region = self.regions[name]
                    if len(region.points) > 0:
                        for trackId in allTrackIds:
                            track = self.tracks[trackId]
                            if region.withinRegion(track.getCenterPoint()):
                                regionTrackIds.append(trackId)
                    else:
                        regionTrackIds.extend(allTrackIds)
                    break
        return regionTrackIds

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
            logging.exception('')            
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
                    print('failed to read track file\n' + trackPath + ';\n' + str(detail))
            del self.regionTrackIds[:]
            self.regionTrackIds.extend(self.tracks.keys())
                        
    def updateAllTracksWorker(self, winCallback, failCallback, progressCallback=None):
        try:
            self.updateLock.acquire()
            self.updateAllTracks(progressCallback)
            winCallback()
        except Exception as detail:
            logging.exception('')            
            failCallback(detail)
        finally:
            self.updateLock.release()
            
    def updateAllTracks(self, progressCallback=None, winCallback=None, failCallback=None):
        if winCallback and failCallback:
            t = Thread(target=self.updateAllTracksWorker, args=(winCallback, failCallback, progressCallback))
            t.daemon=True
            t.start()
        else:
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
                
        
        
                    
            
        
            
        
        
            
            
        
        
        
        
    
    
    