#include <queue>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>

#include "../../ICon7/include/icon7/Debug.hpp"

#include "../../thirdparty/flecs/distr/flecs.h"

#include "../../thirdparty/Collision3D/SpatialPartitioning/include/spatial_partitioning/Dbvt.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/include/spatial_partitioning/BvhMedianSplitHeap.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/include/spatial_partitioning/ThreeStageDbvh.hpp"
#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_AnyOrCompound.hpp"

#include "../include/EntityComponents.hpp"
#include "../include/CollisionFilters.hpp"
#include "../include/EntityComponents.hpp"
#include "../include/Realm.hpp"
#include "../include/CollisionWorld.hpp"

void EnqueueRebuildThreaded(
	std::shared_ptr<std::atomic<bool>> fin,
	std::shared_ptr<spp::BroadphaseBase<spp::Aabb, uint32_t, uint32_t, 0>> dbvh,
	std::shared_ptr<void> data)
{
	static std::atomic<int> size = 0;
	static std::mutex mutex;
	static bool done = false;
	using Pair = std::pair<
		std::shared_ptr<std::atomic<bool>>,
		std::shared_ptr<spp::BroadphaseBase<spp::Aabb, uint32_t, uint32_t, 0>>>;
	static std::queue<Pair> queue;
	if (done == false) {
		done = true;
		std::thread thread = std::thread([]() {
			while (true) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				if (size.load() > 0) {
					Pair p;
					bool has = false;
					{
						std::lock_guard lock(mutex);
						if (queue.empty() == false) {
							p = queue.front();
							queue.pop();
							has = true;
							--size;
						}
					}
					if (has && p.second.use_count() > 1) {
						p.second->Rebuild();
						p.first->store(true);
					}
				}
			}
		});
		thread.detach();
	}
	if (fin.get() != nullptr) {
		std::lock_guard lock(mutex);
		queue.push({fin, dbvh});
		++size;
	}
}

CollisionWorld_spp::CollisionWorld_spp(Realm *realm)
{
	// TODO: Replace this code with some proper *.so compilation configuration
	//       that even more forces to export all symbols than ENABLE_EXPORTS
	this->realm = realm;

// 	spp::ThreeStageDbvh<spp::Aabb, uint32_t, uint32_t, 0> *tsdbvh =
// 		new spp::ThreeStageDbvh<spp::Aabb, uint32_t, uint32_t, 0>(
// 			std::make_shared<
// 				spp::BvhMedianSplitHeap<spp::Aabb, uint32_t, uint32_t, 0, 1>>(
// 				200000),
// 			std::make_shared<
// 				spp::BvhMedianSplitHeap<spp::Aabb, uint32_t, uint32_t, 0, 1>>(
// 				200000),
// 			std::make_unique<
// 				spp::Dbvt<spp::Aabb, uint32_t, uint32_t, 0, uint32_t>>());
// 	tsdbvh->SetRebuildSchedulerFunction(EnqueueRebuildThreaded);
// 	broadphase = tsdbvh;
	broadphase = new spp::BvhMedianSplitHeap<spp::Aabb, uint32_t, uint32_t, 0, 1>(
				200000);
}

CollisionWorld_spp::~CollisionWorld_spp()
{
	Clear();
	delete broadphase;
	broadphase = nullptr;
}

void CollisionWorld_spp::Clear()
{
	broadphase->Clear();
	broadphase->ShrinkToFit();
}

void CollisionWorld_spp::OnStaticCollisionShape(
	flecs::entity entity, const ComponentCollisionShape &shape,
	const ComponentStaticTransform &transform)
{
	spp::Aabb aabb = shape.shape.GetAabb(transform.trans);
	broadphase->Add(entity.id() & 0xFFFFFFFF, aabb, shape.mask);
}

void CollisionWorld_spp::OnAddEntity(flecs::entity entity,
									 const ComponentShape &shape, glm::vec3 pos)
{
	spp::Aabb aabb;
	aabb.min = pos - glm::vec3(shape.width * 0.5f, 0, shape.width * 0.5f);
	aabb.max = aabb.min + glm::vec3(shape.width, shape.height, shape.width);
	broadphase->Add(entity.id() & 0xFFFFFFFF, aabb, FILTER_CHARACTER);
}

void CollisionWorld_spp::OnRemoveEntity(flecs::entity entity)
{
	broadphase->Remove(entity.id() & 0xFFFFFFFF);
}

void CollisionWorld_spp::EntitySetTransform(
	flecs::entity entity, const ComponentStaticTransform &transform,
	const ComponentCollisionShape &shape)
{
	spp::Aabb aabb = shape.shape.GetAabb(transform.trans);
	broadphase->Update(entity.id() & 0xFFFFFFFF, aabb);
}

void CollisionWorld_spp::EntitySetTransform(flecs::entity entity,
											const glm::vec3 &pos,
											const ComponentShape &shape)
{
	spp::Aabb aabb;
	aabb.min = pos - glm::vec3(shape.width * 0.5f, 0, shape.width * 0.5f);
	aabb.max = aabb.min + glm::vec3(shape.width, shape.height, shape.width);
	broadphase->Update(entity.id() & 0xFFFFFFFF, aabb);
}

flecs::entity CollisionWorld_spp::GetAliveEntityGeneration(uint32_t id) const
{
	return realm->ecs.get_alive(realm->Entity(id));
}

bool CollisionWorld_spp::RayTestFirstHit(
	glm::vec3 start, glm::vec3 end, glm::vec3 *hitPosition,
	glm::vec3 *hitNormal, flecs::entity *entity, float *travelFactor,
	uint32_t ignoreEntityId, uint32_t mask) const
{
	struct Cb : public CallbackRayFirstHit {
		const CollisionWorld *cw;
		flecs::entity entity = {};
		uint32_t ignoreEntity;
	} cb;
	cb.cw = this;
	cb.ignoreEntity = ignoreEntityId;

	typedef spp::RayPartialResult (*CbT)(CallbackRay *, uint32_t);
	cb.callback = (CbT) + [](Cb *cb, uint32_t eid) -> spp::RayPartialResult {
		if (eid == cb->ignoreEntity) {
			return {1.0f, false};
		}
		auto cw = cb->cw;
		flecs::entity e = cw->GetAliveEntityGeneration(eid);
		if ((e.is_valid() && e.is_alive()) == false) {
			return {1.0f, false};
		}

		float n = 1.0f;
		glm::vec3 normal;
		bool hit = false;

		if (auto *t = e.try_get<ComponentStaticTransform>()) {
			auto *s = e.try_get<ComponentCollisionShape>();
			if (s == nullptr) {
				LOG_FATAL("entity %lu does not have ComponentCollisionShape "
						  "but is inside CollisionWorld",
						  e.id());
				return {1.0f, false};
			}
			if (s->shape.RayTest(t->trans, *cb, n, normal)) {
				hit = true;
			}
		} else if (auto *t = e.try_get<ComponentMovementState>()) {
			auto *s = e.try_get<ComponentShape>();
			if (s == nullptr) {
				LOG_FATAL("entity %lu does not have ComponentShape but is "
						  "inside CollisionWorld",
						  e.id());
				return {1.0f, false};
			}
			Collision3D::Transform tr{t->pos, {0}};
			Collision3D::Cylinder cyl{s->height, s->width * 0.5f};

			if (cyl.RayTest(tr, *cb, n, normal)) {
				hit = true;
			}
		} else {
			LOG_FATAL("entity %lu does not have ComponentStaticTransform nor "
					  "ComponentMovementState but is inside CollisionWorld",
					  e.id());
			return {1.0f, false};
		}

		if (hit == false) {
			return {1.0f, false};
		}

		if (n < 0.0f) {
			n = 0.0f;
		}
		bool res = false;

		if (cb->hasHit == false) {
			if (n < cb->cutFactor) {
				res = true;
			}
		} else if (n < cb->cutFactor || (n - 0.000001f < cb->cutFactor &&
										 (uint32_t)e.id() < cb->hitEntity)) {
			res = true;
		}

		if (res) {
			cb->cutFactor = n;
			cb->hitPoint = cb->start + cb->dir * n;
			cb->hitEntity = e;
			cb->hasHit = true;
			cb->entity = e;
			cb->hitNormal = normal;
			return {n, true};
		}

		return {1.0f, false};
	};

	cb.mask = mask;
	cb.start = start;
	cb.end = end;
	cb.initedVars = false;
	broadphase->IntersectRay(cb);

	if (cb.hasHit && cb.hitEntity && cb.entity.is_alive()) {
		if (hitNormal)
			*hitNormal = cb.hitNormal;
		if (travelFactor)
			*travelFactor = cb.cutFactor;
		if (entity)
			*entity = cb.entity;
		if (hitPosition)
			*hitPosition = cb.hitPoint;
		return true;
	}

	return false;
}

bool CollisionWorld_spp::RayTestFirstHitTerrain(glm::vec3 start, glm::vec3 end,
												glm::vec3 *hitPosition,
												glm::vec3 *hitNormal,
												float *travelFactor,
												flecs::entity *entity) const
{
	return RayTestFirstHit(start, end, hitPosition, hitNormal, entity,
						   travelFactor, 0,
						   FILTER_STATIC_OBJECT | FILTER_TERRAIN);
}

bool CollisionWorld_spp::RayTestFirstHitTerrainVector(
	glm::vec3 start, glm::vec3 toEnd, glm::vec3 *hitPosition,
	glm::vec3 *hitNormal, float *travelFactor, flecs::entity *entity) const
{
	return RayTestFirstHitTerrain(start, start + toEnd, hitPosition, hitNormal,
								  travelFactor, entity);
}

size_t
CollisionWorld_spp::TestForEntitiesAABB(glm::vec3 min, glm::vec3 max,
										std::vector<flecs::entity> *entities,
										uint32_t mask) const
{
	struct Cb : public CallbackAabb {
		std::vector<flecs::entity> *entities;
		const CollisionWorld *cw;
		uint32_t count = 0;
	} cb;
	cb.cw = this;
	cb.entities = entities;
	typedef void (*CbT)(CallbackAabb *, uint32_t);
	cb.callback = (CbT) + [](Cb *cb, uint32_t eid) {
		auto cw = cb->cw;
		flecs::entity e = cw->GetAliveEntityGeneration(eid);
		if ((e.is_valid() && e.is_alive()) == false) {
			return;
		}
		if (auto *t = e.try_get<ComponentStaticTransform>()) {
			auto *s = e.try_get<ComponentCollisionShape>();
			if (s == nullptr) {
				LOG_FATAL("entity %lu does not have ComponentCollisionShape "
						  "but is inside CollisionWorld",
						  e.id());
				return;
			}
			if (cb->IsRelevant(s->shape.GetAabb(t->trans))) {
				cb->entities->push_back(e);
				cb->count++;
			}
		} else if (auto *t = e.try_get<ComponentMovementState>()) {
			auto *s = e.try_get<ComponentShape>();
			if (s == nullptr) {
				LOG_FATAL("entity %lu does not have ComponentShape but is "
						  "inside CollisionWorld",
						  e.id());
				return;
			}
			Collision3D::Transform tr{t->pos, {0}};
			Collision3D::Cylinder cyl{s->height, s->width * 0.5f};
			if (cb->IsRelevant(cyl.GetAabb(tr))) {
				cb->entities->push_back(e);
				cb->count++;
			}
		} else {
			LOG_FATAL("entity %lu does not have ComponentStaticTransform nor "
					  "ComponentMovementState but is inside CollisionWorld",
					  e.id());
		}
	};

	cb.mask = mask;
	cb.aabb = {min, max};
	broadphase->IntersectAabb(cb);

	return cb.count;
}

size_t CollisionWorld_spp::TestForEntitiesAABBApproximate(
	glm::vec3 min, glm::vec3 max, std::vector<flecs::entity> *entities,
	uint32_t mask) const
{
	struct Cb : public CallbackAabb {
		std::vector<flecs::entity> *entities;
		const CollisionWorld *cw;
		uint32_t count = 0;
	} cb;
	cb.cw = this;
	cb.entities = entities;
	typedef void (*CbT)(CallbackAabb *, uint32_t);
	cb.callback = (CbT) + [](Cb *cb, uint32_t eid) {
		auto cw = cb->cw;
		flecs::entity e = cw->GetAliveEntityGeneration(eid);
		if (e.is_valid() && e.is_alive()) {
			cb->entities->push_back(e);
			cb->count++;
		}
	};

	cb.mask = mask;
	cb.aabb = {min, max};
	broadphase->IntersectAabb(cb);

	return cb.count;
}

size_t CollisionWorld_spp::TestForEntitiesSphere(
	glm::vec3 center, float radius, std::vector<flecs::entity> *testedEntities,
	uint32_t mask) const
{
	size_t c = TestForEntitiesAABB(center - radius, center + radius,
								   testedEntities, mask);

	assert(c <= testedEntities->size());

	const float rad2 = radius * radius;

	for (int i = testedEntities->size() - c; i < c; ++i) {
		flecs::entity e = (*testedEntities)[i];
		bool remove = true;
		if (auto *t = e.try_get<ComponentStaticTransform>()) {
			if (auto *s = e.try_get<ComponentCollisionShape>()) {
				spp::Aabb aabb = s->shape.GetAabb(t->trans);
				glm::vec3 c = aabb.GetCenter();
				glm::vec3 v = c - center;
				if (glm::dot(v, v) <= rad2) {
					remove = false;
				}
			}
		}
		if (auto *t = e.try_get<ComponentMovementState>()) {
			if (auto *s = e.try_get<ComponentShape>()) {
				glm::vec3 m = {s->width, 0, s->width};
				spp::Aabb aabb = {t->pos-m, t->pos+m};
				aabb.min.y += s->height;
				glm::vec3 c = aabb.GetCenter();
				glm::vec3 v = c - center;
				if (glm::dot(v, v) <= rad2) {
					remove = false;
				}
			}
		}
		if (remove) {
			(*testedEntities)[i] =
				(*testedEntities)[testedEntities->size() - 1];
			testedEntities->resize(testedEntities->size() - 1);
			--c;
			--i;
		}
	}

	return c;
}

size_t CollisionWorld_spp::TestForEntitiesCylinder(
	glm::vec3 centerBottom, float radius, float height,
	std::vector<flecs::entity> *testedEntities, uint32_t mask) const
{
	size_t c = TestForEntitiesAABB(
		centerBottom - glm::vec3(radius, 0, radius),
		centerBottom + glm::vec3(radius, height, radius), testedEntities, mask);

	assert(c <= testedEntities->size());

	const float rad2 = radius * radius;

	for (int i = testedEntities->size() - c; i < c; ++i) {
		flecs::entity e = (*testedEntities)[i];
		bool remove = true;
		if (auto *t = e.try_get<ComponentStaticTransform>()) {
			if (auto *s = e.try_get<ComponentCollisionShape>()) {
				spp::Aabb aabb = s->shape.GetAabb(t->trans);
				glm::vec3 c = aabb.GetCenter();
				glm::vec3 v = c - centerBottom;
				v.y = 0;
				if (glm::dot(v, v) <= rad2 && c.y >= centerBottom.y &&
					c.z <= centerBottom.y + height) {
					remove = false;
				}
			}
		}
		if (remove) {
			(*testedEntities)[i] =
				(*testedEntities)[testedEntities->size() - 1];
			testedEntities->resize(testedEntities->size() - 1);
			--c;
			--i;
		}
	}

	return c;
}

void CollisionWorld_spp::StartEpoch() {}
void CollisionWorld_spp::EndEpoch() {
	broadphase->Rebuild();
}

void CollisionWorld_spp::RegisterObservers(Realm *realm)
{
	auto &ecs = realm->ecs;
	ecs.observer<ComponentShape, ComponentMovementState>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, const ComponentShape &shape,
					 const ComponentMovementState &state) {
			OnAddEntity(entity, shape, state.pos);
		});

	ecs.observer<ComponentShape, ComponentMovementState>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity, const ComponentShape &shape,
					 const ComponentMovementState &state) {
			assert(entity.has<ComponentMovementState>());
			EntitySetTransform(entity, state.pos, shape);
		});

	ecs.observer<ComponentShape, ComponentMovementState>()
		.event(flecs::OnRemove)
		.each(
			[this](flecs::entity entity, const ComponentShape &,
				   const ComponentMovementState &) { OnRemoveEntity(entity); });

	ecs.observer<ComponentCollisionShape, ComponentStaticTransform>()
		.event(flecs::OnAdd)
		.each([this](flecs::entity entity, const ComponentCollisionShape &shape,
					 const ComponentStaticTransform &transform) {
			OnStaticCollisionShape(entity, shape, transform);
		});

	ecs.observer<ComponentStaticTransform, ComponentCollisionShape>()
		.event(flecs::OnSet)
		.each([this](flecs::entity entity,
					 const ComponentStaticTransform &transform,
					 const ComponentCollisionShape &shape) {
			EntitySetTransform(entity, transform, shape);
		});

	ecs.observer<ComponentStaticTransform, ComponentCollisionShape>()
		.event(flecs::OnRemove)
		.each([this](flecs::entity entity, const ComponentStaticTransform &,
					 const ComponentCollisionShape &) {
			OnRemoveEntity(entity);
		});
}

int RegisterEntityComponentsCollisionWorld(flecs::world &ecs) { return 0; }
