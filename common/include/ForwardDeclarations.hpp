#pragma once

#include "../../thirdparty/Collision3D/include/collision3d/ForwardDeclarations.hpp"

namespace bitscpp
{
namespace v2
{
class ByteReader;
class ByteWriter_ByteBuffer;
} // namespace v2
} // namespace bitscpp

namespace icon7
{
class ByteBuffer;
class ByteReader;
class ByteWriter;
} // namespace icon7

class btCollisionShape;
class btCylinderShape;
class btCapsuleShape;
class btSphereShape;
class btBvhTriangleMeshShape;
class btCollisionObject;

class Realm;

struct ComponentCollisionShape;
struct ComponentStaticTransform;
struct ComponentShape;

namespace flecs
{
struct entity;
};
