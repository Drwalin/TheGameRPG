#pragma once

#include "ComponentCallbackRegistry.hpp"

#include "../../common/include/EntityComponents.hpp"

struct ComponentTagCanBeUsed {
	template <typename S> S &__ByteStream_op(S &s) { return s; }
};

struct ComponentOnUse {
	named_callbacks::registry_entries::OnUse *entry = nullptr;

	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s)
	{
		std::string name;
		s.op(name);
		if (s.is_valid()) {
			entry = named_callbacks::registry_entries::OnUse::Get(name);
		} else {
			entry = nullptr;
		}
		return s;
	}
	template <typename BT>
	inline bitscpp::ByteWriter<BT> &__ByteStream_op(bitscpp::ByteWriter<BT> &s)
	{
		if (entry) {
			s.op(entry->shortName);
		} else {
			s.op("");
		}
		return s;
	}
};

struct ComponentSingleDoorTransformStates {
	ComponentStaticTransform transformClosed;
	ComponentStaticTransform transformOpen;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(transformClosed);
		s.op(transformOpen);
		return s;
	}
};
