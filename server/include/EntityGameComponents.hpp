#pragma once

#include <unordered_set>

#include <icon7/ByteBuffer.hpp>

#include "ComponentCallbacks.hpp"

#include "../../common/include/EntityComponents.hpp"

namespace named_callbacks
{
namespace registry_entries
{
struct AiBehaviorTick;
struct OnTriggerEnterExit;
struct OnUse;
} // namespace registry_entries
} // namespace named_callbacks

struct ComponentOnUse {
	named_callbacks::registry_entries::OnUse *entry = nullptr;

	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s);
	bitscpp::ByteWriter<icon7::ByteBuffer> &
	__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s);

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

	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s);
	bitscpp::ByteWriter<icon7::ByteBuffer> &
	__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s);
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

	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s);
	bitscpp::ByteWriter<icon7::ByteBuffer> &
	__ByteStream_op(bitscpp::ByteWriter<icon7::ByteBuffer> &s);

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentAITick, MV(aiTick));
};
