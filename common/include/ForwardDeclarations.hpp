#pragma once

namespace bitscpp
{
template <bool V> class ByteReader;
template <typename T> class ByteWriter;
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
