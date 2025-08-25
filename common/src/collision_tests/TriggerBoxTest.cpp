#include "../../../thirdparty/flecs/include/flecs.h"

#include <icon7/Debug.hpp>

#include "../../include/EntityComponents.hpp"
#include "../../include/CollisionWorld.hpp"

void CollisionWorld::TriggerTestBoxForCharacters(
	flecs::entity entity, std::vector<uint64_t> &entities)
{
	entities.clear();
	
	auto trans = entity.try_get<ComponentStaticTransform>();
	if (trans == nullptr) {
		LOG_ERROR(
			"No ComponentStaticTransform found for trigger entity %lu", entity.id());
		return;
	}
	
	auto shape = entity.try_get<ComponentCollisionShape>();
	if (shape == nullptr) {
		LOG_ERROR(
			"No ComponentCollisionShape found for trigger entity %lu", entity.id());
		return;
	}
	
	spp::Aabb aabb = shape->shape.GetAabb(trans->trans);
	
	struct Cb : public CallbackAabb {
		std::vector<uint64_t> *objects;
		const CollisionWorld *cw;
		const ComponentStaticTransform *triggerTrans;
		const ComponentCollisionShape *triggerShape;
	} cb;
	cb.triggerTrans = trans;
	cb.triggerShape = shape;
	cb.cw = this;
	cb.objects = &entities;
	typedef void (*CbT)(CallbackAabb *, uint32_t);
	cb.callback = (CbT) + [](Cb *cb, uint32_t eid) {
		auto cw = cb->cw;
		flecs::entity e = cw->GetAliveEntityGeneration(eid);
		if (auto *t = e.try_get<ComponentMovementState>()) {
			auto *s = e.try_get<ComponentShape>();
			if (s == nullptr) {
				LOG_FATAL("entity %lu does not have ComponentShape but is inside CollisionWorld", e.id());
				return;
			}
			Collision3D::RayInfo ray;
			ray.Calc(t->pos + glm::vec3(0, s->height, 0), t->pos);
			
			float moveFactor = 0;
			glm::vec3 normal;
			if (cb->triggerShape->shape.RayTest(cb->triggerTrans->trans, ray, moveFactor, normal)) {
				cb->objects->push_back(e);
			}
		} else {
			LOG_FATAL("entity %lu does not have ComponentStaticTransform nor ComponentMovementState but is inside CollisionWorld", e.id());
		}
	};
	
	cb.mask = FILTER_CHARACTER;
	cb.aabb = aabb;
	
	broadphase->IntersectAabb(cb);
}
