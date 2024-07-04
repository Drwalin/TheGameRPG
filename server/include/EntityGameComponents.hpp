#pragma once

#include <unordered_set>

#include "ComponentCallbackRegistry.hpp"

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

struct ComponentTrigger {
	named_callbacks::registry_entries::OnTriggerEnterExit *onEnter = nullptr;
	named_callbacks::registry_entries::OnTriggerEnterExit *onExit = nullptr;

	std::unordered_set<uint64_t> entitiesInside = {};
	int64_t tickUntilIgnore = 0;

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

	inline ComponentTeleport(std::string realmName, glm::vec3 position)
	{
// 		LOG_INFO("\t\tctor_args \t\t%p", this);
		this->realmName = realmName;
		this->position = position;
	}

	inline ComponentTeleport()
	{
// 		LOG_INFO("\t\tctor \t\t%p", this);
		realmName = "";
	}
	inline ~ComponentTeleport() {
// 		LOG_INFO("\t\tdtor \t\t%p", this);
	}
	inline ComponentTeleport(const ComponentTeleport &o)
		: realmName(o.realmName), position(o.position)
	{
// 		LOG_INFO("\t\tctor_copy \t%p (%p)", this, &o);
	}
	inline ComponentTeleport(ComponentTeleport &o)
		: realmName(o.realmName), position(o.position)
	{
// 		LOG_INFO("\t\tctor2_copy \t%p (%p)", this, &o);
	}
	inline ComponentTeleport(ComponentTeleport &&o)
		: realmName(std::move(o.realmName)), position(o.position)
	{
// 		LOG_INFO("\t\tctor_move \t%p (%p)", this, &o);
	}

	inline ComponentTeleport &operator=(const ComponentTeleport &o)
	{
// 		LOG_INFO("\t\t\tcopy \t\t%p (%p)", this, &o);
		realmName = o.realmName;
		position = o.position;
		return *this;
	}
	inline ComponentTeleport &operator=(ComponentTeleport &o)
	{
// 		LOG_INFO("\t\t\t\tcopy2 \t\t%p (%p)", this, &o);
		realmName = o.realmName;
		position = o.position;
		return *this;
	}
	inline ComponentTeleport &operator=(ComponentTeleport &&o)
	{
// 		LOG_INFO("\t\t\tmove \t\t%p (%p)", this, &o);
		realmName = o.realmName;
		position = o.position;
		return *this;
	}
};
