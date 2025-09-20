#pragma once

#include <tuple>

#include "../../ICon7/include/icon7/Command.hpp"

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
