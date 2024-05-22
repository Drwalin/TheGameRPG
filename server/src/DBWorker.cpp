
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
	if (rc != SQLITE_OK) {
		LOG_FATAL("Failed to open sqlite database file `%s`, reason: %s",
				  databaseFileName.c_str(), sqlite3_errmsg(db));
		sqlite3_close(db);
		db = nullptr;
		return false;
	}
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

void DBWorker::RunAsync() { queue.RunAsyncExecution(128, 2048); }

void DBWorker::QueueStopRunning() { queue.QueueStopAsyncExecution(); }

void DBWorker::WaitStopAsyncExecution() { queue.WaitStopAsyncExecution(); }

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
	if (rc != SQLITE_OK) {
		errMsg = _errMsg;
		sqlite3_free(_errMsg);
	}
	return rc;
}

int DBWorker::ExecutePrintError(const char *sql)
{
	char *errMsg = nullptr;
	int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		LOG_ERROR("Sqlite error: %s", errMsg);
		sqlite3_free(errMsg);
	}
	return rc;
}

int DBWorker::PrepareStatement(const char *sql, sqlite::Statement *stmt)
{
	stmt->stmt = nullptr;
	int rc = sqlite3_prepare_v3(db, sql, -1, SQLITE_PREPARE_PERSISTENT,
								&stmt->stmt, 0);
	if (rc != SQLITE_OK) {
		LOG_ERROR("Sqlite3 error [%i]: %s", rc, sqlite3_errmsg(db));
		sqlite3_finalize(stmt->stmt);
		stmt->stmt = nullptr;
	} else {
		stmt->db = db;
	}
	return rc;
}

void DBWorker::InitDatabaseStructure()
{
	ExecutePrintError("PRAGMA journal_mode = WAL;");
	ExecutePrintError("PRAGMA synchronous = NORMAL;");
	ExecutePrintError("PRAGMA encoding = 'UTF-8';");
	ExecutePrintError("PRAGMA locking_mode = EXCLUSIVE;");
	ExecutePrintError("PRAGMA cache_size = 4096;");

	ExecutePrintError("CREATE TABLE IF NOT EXISTS Realms("
					  "name TEXT PRIMARY KEY,"
					  "collision_data BLOB,"
					  "graphics_model BLOB) STRICT;");

	ExecutePrintError("CREATE TABLE IF NOT EXISTS Users("
					  "id UNSIGNED BIG INT PRIMARY KEY AUTOINCREMENT,"
					  "username TEXT NOT NULL,"
					  "realm_name TEXT,"
					  "entity_id UNSIGNED BIG INT NOT NULL,"
					  "movement_state BLOB,"
					  "character_sheet_static BLOB,"
					  "character_sheet_dynamic BLOB,"
					  "inventory BLOB,"
					  "equipped_inventory BLOB,"
					  "FOREIGN KEY(realm_name) REFERENCES realm(name)"
					  ") STRICT;");

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

	ExecutePrintError(
		"CREATE UNIQUE INDEX Users_username_index ON Users(username);");
	ExecutePrintError("CREATE UNIQUE INDEX Users_id_index ON Users(id);");
}
