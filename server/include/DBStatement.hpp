#pragma once

#include <cstdint>

#include <string_view>

namespace sqlite
{
enum ErrorCode : int {
	OK = 0,
	ERROR = 1,
	INTERNAL = 2,
	PERM = 3,
	ABORT = 4,
	BUSY = 5,
	LOCKED = 6,
	NOMEM = 7,
	READONLY = 8,
	INTERRUPT = 9,
	IOERR = 10,
	CORRUPT = 11,
	NOTFOUND = 12,
	FULL = 13,
	CANTOPEN = 14,
	PROTOCOL = 15,
	EMPTY = 16,
	SCHEMA = 17,
	TOOBIG = 18,
	CONSTRAINT = 19,
	MISMATCH = 20,
	MISUSE = 21,
	NOLFS = 22,
	AUTH = 23,
	FORMAT = 24,
	RANGE = 25,
	NOTADB = 26,
	NOTICE = 27,
	WARNING = 28,
	ROW = 100,
	DONE = 101,
};
}

struct sqlite3;
struct sqlite3_stmt;

namespace sqlite
{
struct Statement
{
	sqlite3 *db = nullptr;
	sqlite3_stmt *stmt = nullptr;
	
	int Destroy();
	int Step();
	int Reset();
	
	inline bool IsValid() const { return db && stmt; }
	inline operator bool() const { return db && stmt; }
	
	int BindString(int id, std::string_view value);
	int BindBlob(int id, std::string_view value);
	int BindInt32(int id, int32_t value);
	int BindInt64(int id, int64_t value);
	int BindFloat(int id, double value);
	int BindNull(int id);
	
	int ClearBindings();
	
	int ColumnCount();
	std::string_view ColumnName(int id);
	
	bool IsString(int id);
	bool IsBlob(int id);
	bool IsInt(int id);
	bool IsFloat(int id);
	bool IsNull(int id);
	
	std::string_view ColumnString(int id);
	std::string_view ColumnBlob(int id);
	int32_t ColumnInt32(int id);
	int64_t ColumnInt64(int id);
	double ColumnFloat(int id);
};
}
