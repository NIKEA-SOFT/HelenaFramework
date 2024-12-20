﻿// ╔═╗  ╔═╗ ╔══════╗ ╔═╗      ╔══════╗ ╔══╗   ╔═╗  ╔════╗
// ║ ║  ║ ║ ║ ╔════╝ ║ ║      ║ ╔════╝ ║  ╚╗  ║ ║ ╔╝╔══╗╚╗
// ║ ╚══╝ ║ ║ ╚══╗   ║ ║      ║ ╚══╗   ║ ╔╗╚╗ ║ ║ ║ ╚══╝ ║
// ║ ╔══╗ ║ ║ ╔══╝   ║ ║      ║ ╔══╝   ║ ║╚╗╚╗║ ║ ║ ╔══╗ ║
// ║ ║  ║ ║ ║ ╚════╗ ║ ╚════╗ ║ ╚════╗ ║ ║ ╚╗╚╝ ║ ║ ║  ║ ║
// ╚═╝  ╚═╝ ╚══════╝ ╚══════╝ ╚══════╝ ╚═╝  ╚═══╝ ╚═╝  ╚═╝
// Author: NSOFT
// GitHub: https://github.com/NIKEA-SOFT

#ifndef HELENA_HPP
#define HELENA_HPP

// Version
#define HELENA_VERSION_MAJOR 4
#define HELENA_VERSION_MINOR 0
#define HELENA_VERSION_PATCH 0

// Platform
#include <Helena/Platform/Assert.hpp>
#include <Helena/Platform/Compiler.hpp>
#include <Helena/Platform/Defines.hpp>
#include <Helena/Platform/Platform.hpp>
#include <Helena/Platform/Processor.hpp>

// Engine
#include <Helena/Engine/Events.hpp>
#include <Helena/Engine/Engine.hpp>

// Logging
#include <Helena/Logging/Internal/LoggerBuffer.hpp>
#include <Helena/Logging/ColorStyle.hpp>
#include <Helena/Logging/CustomPrint.hpp>
#include <Helena/Logging/Formatter.hpp>
#include <Helena/Logging/FileLogger.hpp>
#include <Helena/Logging/Logging.hpp>
#include <Helena/Logging/Message.hpp>
#include <Helena/Logging/MuteController.hpp>
#include <Helena/Logging/Prefix.hpp>
#include <Helena/Logging/Print.hpp>

// Traits
#include <Helena/Traits/Add.hpp>
#include <Helena/Traits/AnyOf.hpp>
#include <Helena/Traits/Arguments.hpp>
#include <Helena/Traits/Cacheline.hpp>
#include <Helena/Traits/Conditional.hpp>
#include <Helena/Traits/Constness.hpp>
#include <Helena/Traits/Constructible.hpp>
#include <Helena/Traits/FNV1a.hpp>
#include <Helena/Traits/Function.hpp>
#include <Helena/Traits/Identity.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/PowerOf2.hpp>
#include <Helena/Traits/Remove.hpp>
#include <Helena/Traits/SameAll.hpp>
#include <Helena/Traits/SameAs.hpp>
#include <Helena/Traits/ScopedEnum.hpp>
#include <Helena/Traits/Select.hpp>
#include <Helena/Traits/Specialization.hpp>
#include <Helena/Traits/TypeCounter.hpp>
#include <Helena/Traits/TypeIndex.hpp>
#include <Helena/Traits/Underlying.hpp>
#include <Helena/Traits/UniqueTypes.hpp>

// Types
#include <Helena/Types/Allocators.hpp>
#include <Helena/Types/Any.hpp>
#include <Helena/Types/BenchmarkScoped.hpp>
#include <Helena/Types/CompressedPair.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/Delegate.hpp>
#include <Helena/Types/EncryptedString.hpp>
#include <Helena/Types/FixedBuffer.hpp>
#include <Helena/Types/Function.hpp>
#include <Helena/Types/Hash.hpp>
#include <Helena/Types/LocationString.hpp>
#include <Helena/Types/Monostate.hpp>
#include <Helena/Types/Mutex.hpp>
#include <Helena/Types/Overloads.hpp>
#include <Helena/Types/ReferencePointer.hpp>
#include <Helena/Types/RWLock.hpp>
#include <Helena/Types/SourceLocation.hpp>
#include <Helena/Types/Spinlock.hpp>
#include <Helena/Types/SPSCVector.hpp>
#include <Helena/Types/StateMachine.hpp>
#include <Helena/Types/Subsystems.hpp>
#include <Helena/Types/System.hpp>
#include <Helena/Types/TaskScheduler.hpp>
#include <Helena/Types/TimeSpan.hpp>
#include <Helena/Types/UniqueIndexer.hpp>
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorKVAny.hpp>
#include <Helena/Types/VectorUnique.hpp>

// Util
#include <Helena/Util/Cast.hpp>
#include <Helena/Util/Math.hpp>
#include <Helena/Util/Process.hpp>
#include <Helena/Util/String.hpp>

#endif // HELENA_HPP
