#pragma once

/*
 * Server include
 */
#include "../include/ClientRpcProxy.hpp"
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
#include "../../common/include/EntitySystems.hpp"
#include "../../common/include/GlmSerialization.hpp"
#include "../../common/include/Realm.hpp"
#include "../../common/include/RegistryComponent.hpp"
#include "../../common/include/ServerRpcFunctionNames.hpp"
#include "../../common/include/TickTimer.hpp"

/*
 * Library includes
 */
#include "../../thirdparty/flecs/distr/flecs.h"

#include "../../ICon7/include/icon7/ByteBuffer.hpp"
#include "../../ICon7/include/icon7/ByteReader.hpp"
#include "../../ICon7/include/icon7/ByteWriter.hpp"
#include "../../ICon7/include/icon7/Command.hpp"
#include "../../ICon7/include/icon7/CommandExecutionQueue.hpp"
#include "../../ICon7/include/icon7/CommandsBuffer.hpp"
#include "../../ICon7/include/icon7/CommandsBufferHandler.hpp"
#include "../../ICon7/include/icon7/ConcurrentQueueTraits.hpp"
#include "../../ICon7/include/icon7/DNS.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"
#include "../../ICon7/include/icon7/Flags.hpp"
#include "../../ICon7/include/icon7/FrameDecoder.hpp"
#include "../../ICon7/include/icon7/FramingProtocol.hpp"
#include "../../ICon7/include/icon7/Host.hpp"
#include "../../ICon7/include/icon7/HostUStcp.hpp"
#include "../../ICon7/include/icon7/HostUStcpUdp.hpp"
#include "../../ICon7/include/icon7/MemoryPool.hpp"
#include "../../ICon7/include/icon7/MessageConverter.hpp"
#include "../../ICon7/include/icon7/OnReturnCallback.hpp"
#include "../../ICon7/include/icon7/Peer.hpp"
#include "../../ICon7/include/icon7/PeerFlagsArgumentsReader.hpp"
#include "../../ICon7/include/icon7/PeerUStcp.hpp"
#include "../../ICon7/include/icon7/PeerUStcpUdp.hpp"
#include "../../ICon7/include/icon7/RPCEnvironment.hpp"
#include "../../ICon7/include/icon7/Random.hpp"
#include "../../ICon7/include/icon7/SendFrameStruct.hpp"
#include "../../ICon7/include/icon7/Time.hpp"
#include "../../ICon7/include/icon7/Util.hpp"

#include "../../ICon7/bitscpp/include/bitscpp/ByteReader.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteReaderExtensions.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteWriter.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteWriterExtensions.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/Endianness.hpp"
