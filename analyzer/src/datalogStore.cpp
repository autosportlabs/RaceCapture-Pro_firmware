/*
 * datalogStore.cpp
 *
 *  Created on: May 19, 2009
 *      Author: brent
 */
#include "datalogStore.h"
#include "wx/tokenzr.h"
#include "logging.h"

#define DEFAULT_WORKING_PATH "."
#define DEFAULT_DATASTORE_NAME "untitled.radb"
#define DATALOG_ITEM_DELIMITER ","
#define DATALOG_ITEM_STRING_DELIMETER "\""
#define DATALOG_ITEM_STRING_DELIMITER_ESCAPE "\"\""
#define DATALOG_STORE_FILE_EXTENSION ".radb"


#define DATALOG_FILE_COMMENT "#"
#define IMPORT_PROGRESS_COARSENESS 100

DatalogStore::DatalogStore() : m_isOpen(false),m_db(NULL),m_datastoreName(DEFAULT_DATASTORE_NAME){

}

DatalogStore::~DatalogStore(){
	Close();
}

wxString DatalogStore::GetFileName(){
	return m_datastoreName;
}

void DatalogStore::Open(wxString fileName){

	//close if we're already open
	if (m_isOpen) Close();

	SetFilename(fileName);
	VERBOSE(FMT("Opening DataStore %s", m_datastoreName.ToAscii()));
	int rc = sqlite3_open(m_datastoreName.ToAscii(),&m_db);
	if (0 != rc){
		wxString errMsg(sqlite3_errmsg(m_db));
		sqlite3_close(m_db);
		m_isOpen = false;
		throw DatastoreException(errMsg,rc);
	}
	m_isOpen = true;
}

void DatalogStore::Close(){
	if (m_isOpen){
		VERBOSE(FMT("Closing DataStore %s", m_datastoreName.ToAscii()));
		sqlite3_close(m_db);
		m_isOpen = false;
	}
}

bool DatalogStore::IsOpen(){
	return m_isOpen;
}

void DatalogStore::SetFilename(wxString fileName){
	if (! fileName.EndsWith(DATALOG_STORE_FILE_EXTENSION)){
		fileName+=DATALOG_STORE_FILE_EXTENSION;
	}
	m_datastoreName = fileName;
}

void DatalogStore::CreateNew(wxString fileName){
	Open(fileName);
	CreateTables();
}

void DatalogStore::CreateTables(){

	char *sqlErrMsg = NULL;
	{
		const char *CREATE_EVENT_TABLE_SQL= \
		"CREATE TABLE event(name TEXT NOT NULL, notes TEXT NULL, date INTEGER NOT NULL)";
		int rc = sqlite3_exec(m_db, CREATE_EVENT_TABLE_SQL, NULL, NULL, &sqlErrMsg);
		if ( rc != SQLITE_OK ){
			wxString errMsg = wxString::Format("Error creating 'event' table: %s", sqlErrMsg);
			sqlite3_free(sqlErrMsg);
			throw DatastoreException(errMsg, rc);
		}
	}

	{
		const char *CREATE_DATALOG_INFO_TABLE_SQL = \
		"CREATE TABLE datalogInfo(id INTEGER PRIMARY KEY AUTOINCREMENT, timeOffset INTEGER NOT NULL, name TEXT NOT NULL, notes TEXT NULL)";
		int rc = sqlite3_exec(m_db, CREATE_DATALOG_INFO_TABLE_SQL, NULL, NULL, &sqlErrMsg);
		if ( rc != SQLITE_OK ){
			wxString errMsg = wxString::Format("Error creating 'datalogInfo' table: %s", sqlErrMsg);
			sqlite3_free(sqlErrMsg);
			throw DatastoreException(errMsg, rc);
		}
	}

	{
		const char *CREATE_DATALOG_TABLE_SQL = \
		"CREATE TABLE datalog(id INTEGER NOT NULL, timePoint INTEGER NOT NULL)";
		int rc = sqlite3_exec(m_db, CREATE_DATALOG_TABLE_SQL, NULL, NULL, &sqlErrMsg);
		if ( rc != SQLITE_OK ){
			wxString errMsg = wxString::Format("Error creating 'datalog' table: %s", sqlErrMsg);
			sqlite3_free(sqlErrMsg);
			throw DatastoreException(errMsg, rc);
		}
	}

	{
		const char *CREATE_DATALOG_TABLE_INDEX_SQL = \
		"CREATE INDEX datalog_index_id on datalog(id)";
		int rc = sqlite3_exec(m_db, CREATE_DATALOG_TABLE_INDEX_SQL, NULL, NULL, &sqlErrMsg);
		if ( rc != SQLITE_OK ){
			wxString errMsg = wxString::Format("Error creating 'datalog_index_id': %s", sqlErrMsg);
			sqlite3_free(sqlErrMsg);
			throw DatastoreException(errMsg, rc);
		}
	}

	{
		const char *CREATE_DATALOG_TABLE_CHANNEL_TYPES_SQL = \
		"CREATE TABLE channelTypes(id INTEGER PRIMARY KEY, name TEXT NOT NULL, units TEXT NOT NULL, smoothing INTEGER NOT NULL, min REAL NULL, max REAL NULL)";
		int rc = sqlite3_exec(m_db, CREATE_DATALOG_TABLE_CHANNEL_TYPES_SQL, NULL, NULL, &sqlErrMsg);
		if ( rc != SQLITE_OK ){
			wxString errMsg = wxString::Format("Error creating 'channelTypes' table: %s", sqlErrMsg);
			sqlite3_free(sqlErrMsg);
			throw DatastoreException(errMsg, rc);
		}
	}

	{
		const char *CREATE_DATALOG_TABLE_CHANNELS_SQL = \
		"CREATE TABLE channels(id INTEGER PRIMARY KEY, name TEXT NOT NULL, typeId integer NOT NULL, description TEXT NULL)";
		int rc = sqlite3_exec(m_db, CREATE_DATALOG_TABLE_CHANNELS_SQL, NULL, NULL, &sqlErrMsg);
		if ( rc != SQLITE_OK ){
			wxString errMsg = wxString::Format("Error creating 'channels' table: %s", sqlErrMsg);
			sqlite3_free(sqlErrMsg);
			throw DatastoreException(errMsg, rc);
		}
	}

	{
		const char *CREATE_DATALOG_CHANNEL_MAP_SQL = \
		"CREATE TABLE datalogChannelMap(datalogId INTEGER NOT NULL, channelId INTEGER NOT NULL)";
		int rc = sqlite3_exec(m_db, CREATE_DATALOG_CHANNEL_MAP_SQL, NULL, NULL, &sqlErrMsg);
		if (rc != SQLITE_OK ){
			wxString errMsg = wxString::Format("Error creating 'datalogChannelMap' table: %s", sqlErrMsg);
			sqlite3_free(sqlErrMsg);
			throw DatastoreException(errMsg, rc);
		}
	}

}


void DatalogStore::GetDatalogHeaders(wxArrayString &headers, wxFFile &file){

	wxString line;

	//read headers from 1st line
	if (! ReadLine(line, file)){
		throw FileAccessException("Could not read Datalog File header", file.GetName());
	}
	ExtractValues(headers,line);
}

void DatalogStore::ImportDatalogChannelMap(int datalogId, wxArrayInt &channelIds){

	sqlite3_exec(m_db,"BEGIN TRANSACTION",NULL,NULL,NULL);

	const char * INSERT_CHANNEL_TYPE_MAP =
		"INSERT INTO datalogChannelMap (datalogId, channelId) values (?,?)";

	sqlite3_stmt *stmt;

	{
		int rc = sqlite3_prepare(m_db, INSERT_CHANNEL_TYPE_MAP, strlen(INSERT_CHANNEL_TYPE_MAP), &stmt, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to prepare insert channelTypeMap statement", rc);
		}
	}

	size_t count = channelIds.Count();

	for (size_t i = 0; i < count; i++){
		int channelId = channelIds[i];
		{
			int rc = sqlite3_bind_int(stmt, 1, datalogId);
			if (SQLITE_OK != rc){
				throw DatastoreException("Failed to bind datalogId parameter for channelTypeMap", rc);
			}
		}
		{
			int rc = sqlite3_bind_int(stmt, 2, channelId);
			if (SQLITE_OK != rc){
				throw DatastoreException("Failed to bind channelId parameter for channelTypeMap", rc);
			}
		}

		int rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc){
			VERBOSE(FMT("Error inserting: %s %d",sqlite3_errmsg(m_db),rc));
			throw DatastoreException("failed to insert channelTypeMap record" ,rc);
		}
		sqlite3_reset(stmt);
	}

	sqlite3_exec(m_db,"COMMIT",NULL,NULL,NULL);
	sqlite3_finalize(stmt);
}


void DatalogStore::ImportDatalog(const wxString &filePath, const wxString &name, const wxString &notes, DatalogChannels &channels, DatalogChannelTypes &channelTypes, DatalogImportProgressListener *progressListener){

	wxFFile datalogFile;

	if (! datalogFile.Open(filePath)){
		throw FileAccessException("Could not open Datalog File", filePath);
	}

	size_t datalogCount = 0;
	size_t datalogLines = 0;
	if (NULL != progressListener){
		datalogLines = CountFileLines(datalogFile);
		datalogFile.Seek(0,wxFromStart);
	}

	wxArrayString columnNames;
	wxArrayInt selectedColumns;
	GetDatalogHeaders(columnNames,datalogFile);

	//Select the columns we are inserting
	size_t columnNamesCount = columnNames.Count();
	for (size_t i = 0; i < columnNamesCount; i++){
		int id = DatalogChannelUtil::FindChannelIdByName(channels, columnNames[i]);
		selectedColumns.Add(id >= 0 ? 1 : 0);
	}

	int timeOffset = 0;

	sqlite3_exec(m_db,"BEGIN TRANSACTION",NULL,NULL,NULL);

	const char * INSERT_DATALOG_INFO_SQL = "INSERT INTO datalogInfo (timeOffset, name, notes) values (?,?,?)";

	sqlite3_stmt *infoStmt;

	{
		int rc = sqlite3_prepare(m_db, INSERT_DATALOG_INFO_SQL, strlen(INSERT_DATALOG_INFO_SQL), &infoStmt, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to prepare Insert DatalogInfo statement", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(infoStmt,1,timeOffset);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind timeOffset parameter for datalogInfo", rc);
		}
	}
	{
		int rc = sqlite3_bind_text(infoStmt,2,name.ToAscii(), name.Length(), SQLITE_STATIC);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind name parameter for datalogInfo", rc);
		}
	}
	{
		int rc = sqlite3_bind_text(infoStmt,3,notes.ToAscii(), notes.Length(), SQLITE_STATIC);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind notes parameter for datalogInfo", rc);
		}
	}

	int rc = sqlite3_step(infoStmt);
	if (SQLITE_DONE != rc){
		VERBOSE(FMT("error inserting: %s %d",sqlite3_errmsg(m_db),rc));
		throw DatastoreException("failed to insert datalogInfo record" ,rc);
	}

	sqlite3_finalize(infoStmt);

	int datalogId = GetTopDatalogId();

	sqlite3_stmt *insertStmt = CreateDatalogInsertPreparedStatement(columnNames, selectedColumns);

	wxArrayString values;
	int timePoint = 0;
	int logInterval = 100; //TODO detect this from file. in Milliseconds
	wxString line;

	int progressReportCoarseness = datalogLines / IMPORT_PROGRESS_COARSENESS;
	if (progressReportCoarseness == 0) progressReportCoarseness = 1;

	while (ReadLine(line,datalogFile)){
		values.Clear();
		ExtractValues(values, line, &selectedColumns);
		InsertDatalogRow(insertStmt, datalogId, timePoint, values);
		timePoint += logInterval;
		if (NULL != progressListener){
			datalogCount++;
			if (datalogCount % progressReportCoarseness == 0){
				progressListener->UpdateProgress((datalogCount * 100) / datalogLines);
			}
		}
	}
	sqlite3_exec(m_db,"COMMIT",NULL,NULL,NULL);

	sqlite3_finalize(insertStmt);
	if (NULL != progressListener) progressListener->UpdateProgress(100);

}

sqlite3_stmt * DatalogStore::CreateDatalogInsertPreparedStatement(wxArrayString &columns, wxArrayInt &selectedColumns){

	sqlite3_stmt *query;

	size_t count = columns.Count();

	wxString sql = "INSERT INTO datalog(id,timePoint,";
	for (size_t i = 0; i < count; i++){
		if (selectedColumns[i]) sql += columns[i] + ",";
	}
	sql.RemoveLast();

	sql += ") VALUES (?,?,";
	for (size_t i = 0; i < count; i++){
		if (selectedColumns[i])	sql += "?,";
	}
	sql.RemoveLast();
	sql += ")";
	VERBOSE(FMT("Datalog insert prepared statement: %s", sql.ToAscii()));
	{
		int rc = sqlite3_prepare(m_db, sql.ToAscii(), sql.Length(), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to prepare Insert DatalogRow statement", rc);
		}
	}
	return query;
}



void DatalogStore::InsertDatalogRow(sqlite3_stmt *query, int id, int timePoint, wxArrayString &values){

	{
		int rc = sqlite3_bind_int(query,1,id);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind Id for DatalogRow insert",rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query,2,timePoint);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind timePoint for DatalogRow insert",rc);
		}
	}
	size_t count = values.size();
	for (size_t i = 0; i < count; i++){
		double value;
		int paramIndex = i + 3; //param index is 3rd param (1 based)
		int rc;
		wxString &item = values.Item(i);
		if (item.Length() == 0){
			rc = sqlite3_bind_null(query, paramIndex);
		}
		else{
			values.Item(i).ToDouble(&value);
			rc = sqlite3_bind_double(query, paramIndex, value);
		}

		if (SQLITE_OK != rc){
			wxString msg = wxString::Format("Failed to bind parameter %ld value %f for DatalogRow insert",i,value);
			throw DatastoreException(msg,rc);
		}
	}
	{
		int rc = sqlite3_step(query);
		if (SQLITE_DONE != rc){
			VERBOSE(FMT("error inserting: %s %d",sqlite3_errmsg(m_db),rc));
			throw DatastoreException("failed to insert datalog record" ,rc);
		}
	}
	sqlite3_reset(query);
}


void DatalogStore::AddDatalogChannel(int channelId, DatalogChannel &channel){

	wxString &channelName = channel.name;

	VERBOSE(FMT("Adding Datalog Channel '%s'", channelName.ToAscii()));

	const char * ADD_CHANNEL_SQL = "INSERT INTO channels(id,name,typeId,description) values(?,?,?,?)";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, ADD_CHANNEL_SQL, strlen(ADD_CHANNEL_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to prepare Add Channel query", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query,1,channelId);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind channelIdparameter", rc);
		}
	}
	{
		int rc = sqlite3_bind_text(query,2,channelName.ToAscii(),channelName.Length(), SQLITE_STATIC);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind channelName parameter", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query,3,channel.typeId);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind channelTypeId parameter", rc);
		}
	}
	{
		int rc = sqlite3_bind_text(query,4,channel.description.ToAscii(), channel.description.Length(), SQLITE_STATIC);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind description parameter", rc);
		}
	}
	{
		int rc = sqlite3_step(query);
		if (SQLITE_DONE != rc){
			throw DatastoreException("failed to insert into channels" ,rc);
		}
	}
	sqlite3_finalize(query);

	if (! DatalogColumnExists(channelName)){
		VERBOSE(FMT("Altering datalog table to add column '%s'", channelName.ToAscii()));
		{
			const char *ALTER_TABLE_ADD_CHANNEL_SQL="ALTER TABLE datalog ADD COLUMN ? REAL";
			wxString alterTableSQL(ALTER_TABLE_ADD_CHANNEL_SQL);
			alterTableSQL.Replace("?", channelName, true);

			char *sqlErrMsg = NULL;
			int rc = sqlite3_exec(m_db, alterTableSQL, NULL, NULL, &sqlErrMsg);

			if ( rc != SQLITE_OK ){
				wxString errMsg = wxString::Format("Error altering datalog table to add channel '%s'", sqlErrMsg);
				sqlite3_free(sqlErrMsg);
				throw DatastoreException(errMsg, rc);
			}
		}
	}
	VERBOSE(FMT("Done adding datalog channel '%s'", channelName.ToAscii()));
}

bool DatalogStore::DatalogColumnExists(wxString &channelName){

	const char * META_INFO_SQL = "pragma main.table_info(datalog)";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, META_INFO_SQL, strlen(META_INFO_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to create query for table metadata", rc);
		}
	}
	bool found = false;
	while ( sqlite3_step(query) == SQLITE_ROW){
		wxString colName = sqlite3_column_text(query,1);
		if (channelName == colName){
			VERBOSE(FMT("Found existing datalog table column %s", colName.ToAscii()));
			found = true;
			break;
		}
	}
	sqlite3_finalize(query);
	return found;
}

void DatalogStore::ReadDatalogInfo(int datalogId, int &timeOffset, wxString &name, wxString &notes){

	const char *SELECT_DATALOG_INFO = "Select timeOffset, name, notes from datalogInfo where id = ?";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, SELECT_DATALOG_INFO, strlen(SELECT_DATALOG_INFO), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for datalogInfo", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query, 1, datalogId);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind id while querying datalogInfo", rc);
		}
	}
	while( sqlite3_step(query) == SQLITE_ROW){
		timeOffset = sqlite3_column_int(query, 0);
		name = sqlite3_column_text(query,1);
		notes = sqlite3_column_text(query,2);
	}
	sqlite3_finalize(query);
}


void DatalogStore::ReadDatalogIds(wxArrayInt &data){
	const char * SELECT_DATALOG_IDS_SQL = "SELECT id from datalogInfo";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, SELECT_DATALOG_IDS_SQL, strlen(SELECT_DATALOG_IDS_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for datalog Ids", rc);
		}
	}
	while( sqlite3_step(query) == SQLITE_ROW){
		int datalogId = sqlite3_column_int(query, 0);
		data.Add(datalogId);
	}
	sqlite3_finalize(query);
}


void DatalogStore::GetChannelNames(wxArrayString &channelNames){

	const char * CHANNEL_EXISTS_SQL = "SELECT name FROM channels";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, CHANNEL_EXISTS_SQL, strlen(CHANNEL_EXISTS_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for existing datalog channel names", rc);
		}
	}

	while( sqlite3_step(query) == SQLITE_ROW){
		wxString channelName(sqlite3_column_text(query, 0));
		channelNames.Add(channelName);
		VERBOSE(FMT("Found existing channel '%s'", channelName.ToAscii()));
	}

	sqlite3_finalize(query);
}

void DatalogStore::GetChannelTypes(DatalogChannelTypes &channelTypes){

	const char * GET_CHANNEL_TYPES_SQL = "SELECT name, units, smoothing, min, max FROM channelTypes ORDER BY id ASC";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, GET_CHANNEL_TYPES_SQL, strlen(GET_CHANNEL_TYPES_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for channel types names", rc);
		}
	}

	while( sqlite3_step(query) == SQLITE_ROW){

		wxString name = sqlite3_column_text(query,0);
		wxString units = sqlite3_column_text(query,1);
		int smoothing = sqlite3_column_int(query,2);
		double min = sqlite3_column_double(query,3);
		double max = sqlite3_column_double(query,4);

		channelTypes.Add(DatalogChannelType(name, units, smoothing, min, max, 2));
	}

	sqlite3_finalize(query);
}

void DatalogStore::GetChannel(int datalogId, wxString &channelName, DatalogChannel &channel){

	const char * GET_CHANNEL_SQL = "SELECT name, typeId, description FROM channels WHERE id IN (select channelId from datalogChannelMap where datalogId = ?) and name = ?";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, GET_CHANNEL_SQL, strlen(GET_CHANNEL_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for channels", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query, 1, datalogId);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind datalogId parameter for GetChannel", rc);
		}
	}
	{
		int rc = sqlite3_bind_text(query, 2, channelName,-1, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind channelName parameter for GetChannel", rc);
		}
	}

	if ( sqlite3_step(query) == SQLITE_ROW){
		channel.name = sqlite3_column_text(query,0);
		channel.typeId = sqlite3_column_int(query,1);
		channel.description = sqlite3_column_text(query,2);
	}

	sqlite3_finalize(query);
}



void DatalogStore::GetChannels(int datalogId, DatalogChannels &channels){

	const char * GET_CHANNELS_SQL = "SELECT name, typeId, description FROM channels WHERE id IN (select channelId from datalogChannelMap where datalogId = ?)";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, GET_CHANNELS_SQL, strlen(GET_CHANNELS_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for channels", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query, 1, datalogId);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind datalogId parameter for GetChannels", rc);
		}
	}

	while( sqlite3_step(query) == SQLITE_ROW){

		wxString name = sqlite3_column_text(query,0);
		int typeId = sqlite3_column_int(query,1);
		wxString description = sqlite3_column_text(query,2);
		channels.Add(DatalogChannel(name,typeId,description));
	}

	sqlite3_finalize(query);
}

void DatalogStore::GetAllChannels(DatalogChannels &channels){

	const char * GET_CHANNELS_SQL = "SELECT name, typeId, description from channels ORDER BY id ASC";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, GET_CHANNELS_SQL, strlen(GET_CHANNELS_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for all channels", rc);
		}
	}

	while( sqlite3_step(query) == SQLITE_ROW){

		wxString name = sqlite3_column_text(query,0);
		int typeId = sqlite3_column_int(query,1);
		wxString description = sqlite3_column_text(query,2);
		channels.Add(DatalogChannel(name,typeId,description));
	}

	sqlite3_finalize(query);

}

int DatalogStore::GetTopDatalogId(){

	const char * GET_TOP_DATALOG_ID = "SELECT id from datalogInfo order by id DESC limit 1";

	int topId = 0;
	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, GET_TOP_DATALOG_ID, strlen(GET_TOP_DATALOG_ID), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for top datalogId", rc);
		}
	}

	if( sqlite3_step(query) == SQLITE_ROW){
		topId = sqlite3_column_int(query, 0);
		VERBOSE(FMT("Top Id: %d", topId));
	}
	else{
		//VERBOSE("No TimePoint selected!");
	}
	sqlite3_finalize(query);
	return topId;
}

int DatalogStore::GetTopTimePoint(){

	const char * GET_TOP_TIMEPOINT_SQL = "SELECT timePoint from datalog order by timePoint DESC limit 1";

	int topTimePoint = 0;
	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, GET_TOP_TIMEPOINT_SQL, strlen(GET_TOP_TIMEPOINT_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to query for top timePoint", rc);
		}
	}

	if( sqlite3_step(query) == SQLITE_ROW){
		topTimePoint = sqlite3_column_int(query, 0);
		VERBOSE(FMT("Top TimePoint: %d", topTimePoint));
	}
	else{
		//VERBOSE("No TimePoint selected!");
	}
	sqlite3_finalize(query);
	return topTimePoint;
}

size_t DatalogStore::CountFileLines(wxFFile &file){

	wxString buffer;
	buffer.Alloc(1024);

	size_t count = 0;
	while (ReadLine(buffer,file)){
		if (! buffer.StartsWith(DATALOG_FILE_COMMENT)) count++;
	}
	return count;
}

size_t DatalogStore::ReadLine(wxString &buffer, wxFFile &file){

	buffer = "";
	size_t readCount = 0;
	wxChar value;
	while (!file.Eof()){
		file.Read(&value,1);
		if (value == '\n') break;
		buffer.Append(value);
		readCount++;
	}
	VERBOSE(FMT("ReadLine: %s", buffer.ToAscii()));
	return readCount;
}

size_t DatalogStore::ExtractValues(wxArrayString &valueList, wxString &line, wxArrayInt *selectedColumns){

	wxStringTokenizer tok(line, DATALOG_ITEM_DELIMITER, wxTOKEN_RET_EMPTY);

	size_t selectedColumnsCount = 0;
	if (NULL != selectedColumns) selectedColumnsCount = selectedColumns->Count();
	size_t count = 0;
	while (tok.HasMoreTokens()){
		wxString value = tok.NextToken();
		value.Strip(wxString::both);
		//process string value
		size_t len = value.Length();
		if (len > 0 && value[0] == '"' && value[len - 1] == '"'){
			value.Remove(0,1);
			value.RemoveLast(1);
			value.Replace(DATALOG_ITEM_STRING_DELIMITER_ESCAPE, DATALOG_ITEM_STRING_DELIMETER, true);
		}
		if (NULL == selectedColumns || (count < selectedColumnsCount && selectedColumns->Item(count))){
			valueList.Add(value);
			VERBOSE(FMT("Adding value: %s",value.ToAscii()));
		}
		count++;
	}
	return count;
}


void DatalogStore::ReadChannelTypes(DatalogChannelTypes &channelTypes){

	channelTypes.Clear();

	const char *READ_CHANNEL_TYPES_SQL = "SELECT id, name, units, smoothing, min, max from channelTypes ORDER BY id ASC";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, READ_CHANNEL_TYPES_SQL,strlen(READ_CHANNEL_TYPES_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to create query for ReadChannelTypes", rc);
		}
	}
	while( sqlite3_step(query) == SQLITE_ROW){

		wxString typeName(sqlite3_column_text(query, 1));
		wxString typeUnits(sqlite3_column_text(query,2));
		int smoothing = sqlite3_column_int(query,3);
		double min = sqlite3_column_double(query,4);
		double max = sqlite3_column_double(query,5);

		channelTypes.Add(DatalogChannelType(typeName, typeUnits, smoothing, min, max, 2));
	}
	sqlite3_finalize(query);
}

void DatalogStore::ReadChannels(DatalogChannels &channels){

	channels.Clear();

	const char *READ_CHANNELS_SQL = "SELECT id, name, typeId, description FROM channels ORDER BY id ASC";

	sqlite3_stmt *query;
	{
		int rc = sqlite3_prepare(m_db, READ_CHANNELS_SQL,strlen(READ_CHANNELS_SQL), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to create query for ReadChannelTypes", rc);
		}
	}
	while( sqlite3_step(query) == SQLITE_ROW){

		wxString name(sqlite3_column_text(query, 1));
		int typeId = sqlite3_column_int(query,2);
		wxString description(sqlite3_column_text(query,3));

		channels.Add(DatalogChannel(name,typeId, description));
	}
	sqlite3_finalize(query);
}


void DatalogStore::ReadDatalog(DatalogStoreRows &results, int datalogId, wxArrayString &names, int fromTimePoint, int toTimePoint){

	results.Clear();

	if (toTimePoint == TIMEPOINT_REST){
		toTimePoint = GetTopTimePoint();
	}

	const char * READ_CHANNEL_SQL = "SELECT @ FROM datalog WHERE id = ? AND timePoint >= ? AND timePoint <= ? ORDER BY timePoint ASC";

	wxString querySql(READ_CHANNEL_SQL);
	wxString nameList;
	size_t channelCount = names.size();
	for (size_t col = 0; col < channelCount; col++){
		if (col > 0 ) nameList.Append(",");
		nameList.Append(names[col]);
	}
	querySql.Replace("@", nameList, true);

	VERBOSE(FMT("Read Channel SQL: %s", querySql.ToAscii()));

	sqlite3_stmt *query;
	{

		int rc = sqlite3_prepare(m_db, querySql.ToAscii(), querySql.Length(), &query, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to create query for ReadChannels", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query, 1, datalogId);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind datalogId while querying ReadChannel", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query, 2, fromTimePoint);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind fromTimePoint while querying ReadChannel", rc);
		}
	}
	{
		int rc = sqlite3_bind_int(query, 3, toTimePoint);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to bind toTimePoint while querying ReadChannel", rc);
		}
	}
	while( sqlite3_step(query) == SQLITE_ROW){

		DatastoreRow row;
		row.timePoint = sqlite3_column_int(query,0);
		for (size_t channelCol = 0; channelCol < channelCount; channelCol++){
			if (sqlite3_column_type(query,channelCol) == SQLITE_NULL){
				row.values.Add(DatalogValue::NULL_VALUE);
			}
			else{
				row.values.Add(sqlite3_column_double(query,channelCol));
			}
		}
		results.Add(row);
	}
	sqlite3_finalize(query);
}

void DatalogStore::ClearChannelTypes(){

	char *sqlErrMsg = NULL;

	const char *CLEAR_CHANNEL_TYPES_SQL = \
	"DELETE FROM channelTypes";
	int rc = sqlite3_exec(m_db, CLEAR_CHANNEL_TYPES_SQL, NULL, NULL, &sqlErrMsg);
	if ( rc != SQLITE_OK ){
		wxString errMsg = wxString::Format("Error Clearing 'channelTypes' table: %s", sqlErrMsg);
		sqlite3_free(sqlErrMsg);
		throw DatastoreException(errMsg, rc);
	}
}

void DatalogStore::ClearChannels(){

	char *sqlErrMsg = NULL;

	const char *CLEAR_CHANNELS_SQL = \
	"DELETE FROM channels";
	int rc = sqlite3_exec(m_db, CLEAR_CHANNELS_SQL, NULL, NULL, &sqlErrMsg);
	if ( rc != SQLITE_OK ){
		wxString errMsg = wxString::Format("Error Clearing 'channels' table: %s", sqlErrMsg);
		sqlite3_free(sqlErrMsg);
		throw DatastoreException(errMsg, rc);
	}
}

void DatalogStore::ImportChannels(DatalogChannels &channels){

	size_t count = channels.Count();
	for (size_t i = 0; i < count; i++){
		DatalogChannel &channel = channels[i];
		AddDatalogChannel(i,channel);
	}
}


void DatalogStore::ImportChannelTypes(DatalogChannelTypes &channelTypes){

	sqlite3_exec(m_db,"BEGIN TRANSACTION",NULL,NULL,NULL);

	const char * INSERT_DATALOG_CHANNEL_TYPES_SQL =
		"INSERT INTO channelTypes (id,name, units, smoothing, min, max) values (?,?,?,?,?,?)";

	sqlite3_stmt *stmt;

	{
		int rc = sqlite3_prepare(m_db, INSERT_DATALOG_CHANNEL_TYPES_SQL, strlen(INSERT_DATALOG_CHANNEL_TYPES_SQL), &stmt, NULL);
		if (SQLITE_OK != rc){
			throw DatastoreException("Failed to prepare insert channels statement", rc);
		}
	}

	size_t count = channelTypes.Count();

	for (size_t i = 0; i < count; i++){
		DatalogChannelType &channelType = channelTypes[i];
		{
			int rc = sqlite3_bind_int(stmt, 1, i);
			if (SQLITE_OK != rc){
				throw DatastoreException("Failed to bind id parameter for datalogChannelType", rc);
			}
		}
		{
			int rc = sqlite3_bind_text(stmt, 2, channelType.name.ToAscii(), channelType.name.Length(), SQLITE_STATIC);
			if (SQLITE_OK != rc){
				throw DatastoreException("Failed to bind name parameter for datalogChannelType", rc);
			}
		}
		{
			int rc = sqlite3_bind_text(stmt, 3, channelType.unitsLabel.ToAscii(), channelType.unitsLabel.Length(), SQLITE_STATIC);
			if (SQLITE_OK != rc){
				throw DatastoreException("Failed to bind units parameter for datalogChannelType", rc);
			}
		}
		{
			int rc = sqlite3_bind_int(stmt, 4, channelType.smoothingLevel);
			if (SQLITE_OK != rc){
				throw DatastoreException("Failed to bind smoothing level parameter for datalogChannelType", rc);
			}
		}
		{
			int rc = sqlite3_bind_int(stmt, 5, channelType.minValue);
			if (SQLITE_OK != rc){
				throw DatastoreException("Failed to bind minValue parameter for datalogChannelType", rc);
			}
		}
		{
			int rc = sqlite3_bind_int(stmt, 6, channelType.maxValue);
			if (SQLITE_OK != rc){
				throw DatastoreException("Failed to bind maxValue parameter for datalogChannelType", rc);
			}
		}

		int rc = sqlite3_step(stmt);
		if (SQLITE_DONE != rc){
			VERBOSE(FMT("error inserting: %s %d",sqlite3_errmsg(m_db),rc));
			throw DatastoreException("failed to insert datalogChannelType record" ,rc);
		}
		sqlite3_reset(stmt);
	}

	sqlite3_exec(m_db,"COMMIT",NULL,NULL,NULL);

	sqlite3_finalize(stmt);
}
