#pragma once

#include <unordered_set>

#include "ComponentCallbacks.hpp"

#include "../../common/include/EntityComponents.hpp"

struct ComponentOnUse {
	named_callbacks::registry_entries::OnUse *entry = nullptr;

	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s)
	{
		entry->Deserialize(&entry, s);
		return s;
	}
	template <typename BT>
	inline bitscpp::ByteWriter<BT> &__ByteStream_op(bitscpp::ByteWriter<BT> &s)
	{
		entry->Serialize(&entry, s);
		return s;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentOnUse, MV(entry));
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

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentSingleDoorTransformStates,
								  {MV(transformClosed) MV(transformOpen)});
};

struct ComponentTrigger {
	named_callbacks::registry_entries::OnTriggerEnterExit *onEnter = nullptr;
	named_callbacks::registry_entries::OnTriggerEnterExit *onExit = nullptr;

	std::unordered_set<uint64_t> entitiesInside = {};
	int64_t tickUntilIgnore = 0;

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentTrigger,
								  {MV(onEnter) MV(onExit) MV(entitiesInside)
									   MV(tickUntilIgnore)});

	void Tick(int64_t entityId, class RealmServer *realm);

	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s)
	{
		onEnter->Deserialize(&onEnter, s);
		onExit->Deserialize(&onExit, s);
		return s;
	}
	template <typename BT>
	inline bitscpp::ByteWriter<BT> &__ByteStream_op(bitscpp::ByteWriter<BT> &s)
	{
		onEnter->Serialize(&onEnter, s);
		onExit->Serialize(&onExit, s);
		return s;
	}
};

struct ComponentTeleport {
	std::string realmName = "";
	glm::vec3 position;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(realmName);
		s.op(position);
		return s;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentTeleport,
								  {MV(realmName) MV(position)});
};

struct ComponentAITick {
	named_callbacks::registry_entries::AiBehaviorTick *aiTick = nullptr;

	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s)
	{
		aiTick->Deserialize(&aiTick, s);
		return s;
	}
	template <typename BT>
	inline bitscpp::ByteWriter<BT> &__ByteStream_op(bitscpp::ByteWriter<BT> &s)
	{
		aiTick->Serialize(&aiTick, s);
		return s;
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentAITick, MV(aiTick));
};
