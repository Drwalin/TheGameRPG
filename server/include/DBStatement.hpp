#pragma once

#include <cstdint>

#include <string_view>

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
	
	int BindString(int id, std::string_view value);
	int BindBlob(int id, std::string_view value);
	int BindInt32(int id, int32_t value);
	int BindInt64(int id, int64_t value);
	int BindFloat(int id, double value);
	int BindNull(int id);
	
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
