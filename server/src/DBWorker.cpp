
#include "../../ICon7/include/icon7/Debug.hpp"

#include "../sqlite3.45.3/sqlite3.h"

#include "../include/DBWorker.hpp"

DBWorker::DBWorker() { db = nullptr; }

DBWorker::~DBWorker() { Destroy(); }

DBWorker *DBWorker::GetSingleton()
{
	static DBWorker worker;
	return &worker;
}

bool DBWorker::Init(std::string databaseFileName)
{
	Destroy();
	int rc = sqlite3_open(databaseFileName.c_str(), &db);
	if (rc != sqlite::OK) {
		LOG_FATAL("Failed to open sqlite database file `%s`, reason: %s",
				  databaseFileName.c_str(), sqlite3_errmsg(db));
		sqlite3_close(db);
		db = nullptr;
		return false;
	}
	InitDatabaseStructure();
	return true;
}

void DBWorker::Destroy()
{
	WaitStopAsyncExecution();
	if (db) {
		sqlite3_close(db);
		db = nullptr;
	}
	while (true) {
		icon7::CommandHandle<icon7::Command> com;
		if (queue.TryDequeue(com)) {
			com.Destroy();
		} else {
			break;
		}
	}
}

void DBWorker::RunAsync() {
	queue.RunAsyncExecution(128, 4096);
	queue.EnqueueCommand(
		icon7::CommandHandle<CommandFunctor<void (*)()>>::Create(
			+[]() { LOG_INFO("Database thread"); }));
}

void DBWorker::QueueStopRunning() { queue.QueueStopAsyncExecution(); }

void DBWorker::WaitStopAsyncExecution() { queue.WaitStopAsyncExecution(); }

void DBWorker::SyncToDriveLocal() { sqlite3_db_cacheflush(db); }

int DBWorker::ErrorCode() { return sqlite3_errcode(db); }

std::string DBWorker::ErrorMessage() { return sqlite3_errmsg(db); }

std::string DBWorker::ErrorString(int errorCode)
{
	return sqlite3_errstr(errorCode);
}

int DBWorker::Execute(const char *sql)
{
	int rc = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
	return rc;
}

int DBWorker::Execute(const char *sql, std::string &errMsg)
{
	char *_errMsg = nullptr;
	int rc = sqlite3_exec(db, sql, nullptr, nullptr, &_errMsg);
	if (rc != sqlite::OK) {
		errMsg = _errMsg;
		sqlite3_free(_errMsg);
	}
	return rc;
}

int DBWorker::ExecutePrintError(const char *sql)
{
	char *errMsg = nullptr;
	int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
	if (rc != sqlite::OK) {
		LOG_ERROR("Sqlite error: %s\n_> %s", errMsg, sql);
		
		sqlite3_free(errMsg);
	}
	return rc;
}

int DBWorker::PrepareStatement(const char *sql, sqlite::Statement *stmt)
{
	stmt->stmt = nullptr;
	int rc = sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT,
								&stmt->stmt, 0);
	if (rc != sqlite::OK) {
		LOG_ERROR("Sqlite3 error [%i]: %s", rc, sqlite3_errmsg(db));
		sqlite3_finalize(stmt->stmt);
		stmt->stmt = nullptr;
	} else {
		stmt->db = db;
	}
	return rc;
}

sqlite::Statement DBWorker::PrepareStatement(const char *sql, int *errorCode)
{
	sqlite::Statement stmt;
	int rc = PrepareStatement(sql, &stmt);
	if (errorCode)
		*errorCode = rc;
	return stmt;
}

void DBWorker::InitDatabaseStructure()
{
	ExecutePrintError("PRAGMA journal_mode = WAL;");
	ExecutePrintError("PRAGMA synchronous = NORMAL;");
	ExecutePrintError("PRAGMA encoding = 'UTF-8';");
	ExecutePrintError("PRAGMA locking_mode = EXCLUSIVE;");
	ExecutePrintError("PRAGMA cache_size = 4096;");
	
	ExecutePrintError("CREATE TABLE IF NOT EXISTS WorldOptions("
					  "key TEXT PRIMARY KEY,"
					  "value ANY"
					  ") STRICT;");
	
	ExecutePrintError("CREATE TABLE IF NOT EXISTS Realms("
					  "name TEXT PRIMARY KEY,"
					  "collision_data BLOB,"
					  "graphics_model BLOB) STRICT;");
	
	ExecutePrintError("CREATE TABLE IF NOT EXISTS Users("
					  "username TEXT PRIMARY KEY,"
					  "realm_name TEXT,"
					  "movement_state BLOB,"
					  "character_sheet_static BLOB,"
					  "character_sheet_dynamic BLOB,"
					  "inventory BLOB,"
					  "equipped_inventory BLOB,"
					  "FOREIGN KEY(realm_name) REFERENCES realm(name)"
					  ") STRICT;");
	Execute("CREATE UNIQUE INDEX Users_username_index ON Users(username);");
	
	/*
	ExecutePrintError("CREATE TABLE IF NOT EXISTS Creatures("
					  "name TEXT PRIMARY KEY,"
					  "character_sheet BLOB NOT NULL,"
					  "graphics_model BLOB NOT NULL) STRICT;");

	ExecutePrintError("CREATE TABLE IF NOT EXISTS ItemsTemplates("
					  "name TEXT PRIMARY KEY,"
					  "description TEXT,"
					  "default_price TEXT,"
					  "characteristics BLOB,"
					  "graphics_model BLOB) STRICT;");

	ExecutePrintError("CREATE TABLE IF NOT EXISTS CreaturesInRealms("
					  "name TEXT PRIMARY KEY,"
					  "collision_data BLOB,"
					  "graphics_model BLOB) STRICT;");
	*/

	
	SyncToDriveLocal();
}
