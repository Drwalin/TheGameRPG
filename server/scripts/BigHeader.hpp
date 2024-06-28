#pragma once

/*
 * Server include
 */
#include "../include/ClientRpcProxy.hpp"
#include "../include/CollisionLoader.hpp"
#include "../include/CommandParser.hpp"
#include "../include/ComponentCallbackRegistry.hpp"
#include "../include/EntityComponentsServer.hpp"
#include "../include/EntityGameComponents.hpp"
#include "../include/EntityNetworkingSystems.hpp"
#include "../include/FileOperations.hpp"
#include "../include/FunctorCommands.hpp"
#include "../include/PeerData.hpp"
#include "../include/PeerStateTransitions.hpp"
#include "../include/RealmServer.hpp"
#include "../include/RealmWorkThreadedManager.hpp"
#include "../include/ServerCore.hpp"
#include "../include/SharedObject.hpp"
#include "../include/StringUtil.hpp"

/*
 * Common include
 */
#include "../../common/include/ClientRpcFunctionNames.hpp"
#include "../../common/include/CollisionWorld.hpp"
#include "../../common/include/EntityComponents.hpp"
#include "../../common/include/EntityEvent.hpp"
#include "../../common/include/EntityGameComponents.hpp"
#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/GlmBullet.hpp"
#include "../../common/include/GlmSerialization.hpp"
#include "../../common/include/Realm.hpp"
#include "../../common/include/RegistryComponent.hpp"
#include "../../common/include/ServerRpcFunctionNames.hpp"
#include "../../common/include/Timer.hpp"

/*
 * Library includes
 */
#include <flecs.h>

#include <icon7/ByteBuffer.hpp>
#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>
#include <icon7/Command.hpp>
#include <icon7/CommandExecutionQueue.hpp>
#include <icon7/CommandsBuffer.hpp>
#include <icon7/CommandsBufferHandler.hpp>
#include <icon7/ConcurrentQueueTraits.hpp>
#include <icon7/DNS.hpp>
#include <icon7/Debug.hpp>
#include <icon7/Flags.hpp>
#include <icon7/FrameDecoder.hpp>
#include <icon7/FramingProtocol.hpp>
#include <icon7/Host.hpp>
#include <icon7/HostUStcp.hpp>
#include <icon7/HostUStcpUdp.hpp>
#include <icon7/MemoryPool.hpp>
#include <icon7/MessageConverter.hpp>
#include <icon7/OnReturnCallback.hpp>
#include <icon7/Peer.hpp>
#include <icon7/PeerFlagsArgumentsReader.hpp>
#include <icon7/PeerUStcp.hpp>
#include <icon7/PeerUStcpUdp.hpp>
#include <icon7/RPCEnvironment.hpp>
#include <icon7/Random.hpp>
#include <icon7/SendFrameStruct.hpp>
#include <icon7/Time.hpp>
#include <icon7/Util.hpp>

#include <bitscpp/ByteReader.hpp>
#include <bitscpp/ByteReaderExtensions.hpp>
#include <bitscpp/ByteWriter.hpp>
#include <bitscpp/ByteWriterExtensions.hpp>
#include <bitscpp/Endianness.hpp>
