#pragma once

#include "../../ICon7/include/icon7/CommandExecutionQueue.hpp"

#include "DBStatement.hpp"

struct sqlite3;
struct sqlite3_stmt;

template <typename TFunc> struct CommandFunctor : public icon7::Command {
	virtual ~CommandFunctor() override {}
	CommandFunctor() {}
	CommandFunctor(TFunc &&functor) : functor(std::move(functor)) {}

	TFunc functor;
	virtual void Execute() override { functor(); }
};

template <typename TFunc, typename... Args>
struct CommandBinding : public icon7::Command {
	virtual ~CommandBinding() override {}
	CommandBinding() {}

	TFunc functor;
	std::tuple<Args...> args;
	virtual void Execute() override { std::apply(functor, args); }
};

class DBWorker
{
public:
	DBWorker();
	~DBWorker();

	static DBWorker *GetSingleton();

	bool Init(std::string databaseFileName);
	void Destroy();

	void RunAsync();
	void QueueStopRunning();
	void WaitStopAsyncExecution();
	
	void SyncToDriveLocal();

	int ErrorCode();
	std::string ErrorMessage();
	static std::string ErrorString(int errorCode);

	int Execute(const char *sql);
	int ExecutePrintError(const char *sql);
	int Execute(const char *sql, std::string &errMsg);
	int PrepareStatement(const char *sql, sqlite::Statement *stmt);
	sqlite::Statement PrepareStatement(const char *sql, int *errorCode);

	template <typename TFunc, typename... Args>
	void EnqueueCommand(TFunc &&func, Args... args)
	{
		auto com =
			icon7::CommandHandle<CommandBinding<TFunc, Args...>>::Create();
		com->functor = std::move(func);
		com->args = std::make_tuple(std::forward<Args>(args)...);
		queue.EnqueueCommand(std::move(com));
	}

public:
	icon7::CommandExecutionQueue queue;
	struct sqlite3 *db;

private:
	void InitDatabaseStructure();
};
