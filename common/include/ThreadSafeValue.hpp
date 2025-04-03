#pragma once

#include <mutex>

template <typename T> class ThreadSafeValue
{
public:
	ThreadSafeValue() = default;

	ThreadSafeValue(T &&v) : value(std::move(v)) {}
	ThreadSafeValue(T &v) : value(v) {}
	ThreadSafeValue(const T &v) : value(v) {}

	ThreadSafeValue(const ThreadSafeValue<T> &o) { value = o; }

	ThreadSafeValue &operator=(const ThreadSafeValue<T> &o)
	{
		T v = o;
		*this = (T)v;
		return *this;
	}

	ThreadSafeValue(ThreadSafeValue<T> &o) { value = (T)o; }

	ThreadSafeValue &operator=(ThreadSafeValue<T> &o)
	{
		T v = o;
		*this = (T)v;
		return *this;
	}

	ThreadSafeValue &operator=(T &v)
	{
		std::lock_guard lock(mutex);
		value = v;
		return *this;
	}
	ThreadSafeValue &operator=(const T &v)
	{
		std::lock_guard lock(mutex);
		value = v;
		return *this;
	}

	operator T()
	{
		T ret;
		{
			std::lock_guard lock(mutex);
			ret = value;
		}
		return ret;
	}

	operator T() const
	{
		T ret;
		{
			std::lock_guard lock(*(std::mutex *)&mutex);
			ret = value;
		}
		return ret;
	}

	template <typename T2> ThreadSafeValue(ThreadSafeValue<T2> &o)
	{
		value = (T)o;
	}
	template <typename T2> ThreadSafeValue &operator=(ThreadSafeValue<T2> &o)
	{
		T2 v = o;
		*this = (T)v;
		return *this;
	}

	template <typename T2> ThreadSafeValue &operator=(T2 &v)
	{
		std::lock_guard lock(mutex);
		value = v;
		return *this;
	}

	template <typename T2> operator T2()
	{
		T ret;
		{
			std::lock_guard lock(mutex);
			ret = value;
		}
		return ret;
	}

	template <typename T2> operator T2() const
	{
		T ret;
		{
			std::lock_guard lock(*(std::mutex *)&mutex);
			ret = value;
		}
		return ret;
	}

	void StartAccess() { mutex.lock(); }
	bool TryStartAccess() { return mutex.try_lock(); }
	void EndAccess() { mutex.unlock(); }
	T &Access() { return value; }
	T *operator->() { return &value; }
	T &operator*() { return value; }

private:
	ThreadSafeValue(ThreadSafeValue &&) = delete;
	ThreadSafeValue &operator=(ThreadSafeValue &&) = delete;

private:
	std::mutex mutex;
	T value;
};
