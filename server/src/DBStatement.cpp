#include "../sqlite3.45.3/sqlite3.h"

#include "../include/DBStatement.hpp"

namespace sqlite
{
int Statement::Destroy()
{
	int rc = sqlite3_finalize(stmt);
	stmt = nullptr;
	db = nullptr;
	return rc;
}

int Statement::Step()
{
	return sqlite3_step(stmt);
}

int Statement::Reset()
{
	return sqlite3_reset(stmt);
}

int Statement::BindString(int id, std::string_view value)
{
	return sqlite3_bind_text(stmt, id, value.data(), value.length(), nullptr);
}

int Statement::BindBlob(int id, std::string_view value)
{
	return sqlite3_bind_blob(stmt, id, value.data(), value.length(), nullptr);
}

int Statement::BindInt32(int id, int32_t value)
{
	return sqlite3_bind_int(stmt, id, value);
}

int Statement::BindInt64(int id, int64_t value)
{
	return sqlite3_bind_int64(stmt, id, value);
}

int Statement::BindFloat(int id, double value)
{
	return sqlite3_bind_double(stmt, id, value);
}

int Statement::BindNull(int id)
{
	return sqlite3_bind_null(stmt, id);
}

int Statement::ColumnCount()
{
	return sqlite3_column_count(stmt);
}

std::string_view Statement::ColumnName(int id)
{
	return sqlite3_column_name(stmt, id);
}

bool Statement::IsString(int id)
{
	return sqlite3_column_type(stmt, id) == SQLITE_TEXT;
}

bool Statement::IsBlob(int id)
{
	return sqlite3_column_type(stmt, id) == SQLITE_BLOB;
}

bool Statement::IsInt(int id)
{
	return sqlite3_column_type(stmt, id) == SQLITE_INTEGER;
}

bool Statement::IsFloat(int id)
{
	return sqlite3_column_type(stmt, id) == SQLITE_FLOAT;
}

bool Statement::IsNull(int id)
{
	return sqlite3_column_type(stmt, id) == SQLITE_NULL;
}

std::string_view Statement::ColumnString(int id)
{
	int bytes = sqlite3_column_bytes(stmt, id);
	const unsigned char *data = sqlite3_column_text(stmt, id);
	return std::string_view((char *)data, bytes);
}

std::string_view Statement::ColumnBlob(int id)
{
	int bytes = sqlite3_column_bytes(stmt, id);
	const void *data = sqlite3_column_blob(stmt, id);
	return std::string_view((char *)data, bytes);
}

int32_t Statement::ColumnInt32(int id)
{
	return sqlite3_column_int(stmt, id);
}

int64_t Statement::ColumnInt64(int id)
{
	return sqlite3_column_int64(stmt, id);
}

double Statement::ColumnFloat(int id)
{
	return sqlite3_column_double(stmt, id);
}
}
