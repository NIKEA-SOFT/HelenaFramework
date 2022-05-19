#ifndef HELENA_HPP
#define HELENA_HPP

// Version
#define HELENA_VERSION_MAJOR 1
#define HELENA_VERSION_MINOR 0
#define HELENA_VERSION_PATCH 0

// Platform
#include <Helena/Platform/Platform.hpp>
#include <Helena/Platform/Processor.hpp>
#include <Helena/Platform/Compiler.hpp>
#include <Helena/Platform/Defines.hpp>
#include <Helena/Platform/Assert.hpp>

// Engine
#include <Helena/Engine/Events.hpp>
#include <Helena/Engine/Engine.hpp>
#include <Helena/Engine/Log.hpp>

// Traits
#include <Helena/Traits/AnyOf.hpp>
#include <Helena/Traits/Cacheline.hpp>
#include <Helena/Traits/Constness.hpp>
#include <Helena/Traits/CVRefPtr.hpp>
#include <Helena/Traits/Detector.hpp>
#include <Helena/Traits/FNV1a.hpp>
#include <Helena/Traits/FunctionInfo.hpp>
#include <Helena/Traits/IntegralConstant.hpp>
#include <Helena/Traits/Map.hpp>
#include <Helena/Traits/NameOf.hpp>
#include <Helena/Traits/Pair.hpp>
#include <Helena/Traits/PowerOf2.hpp>
#include <Helena/Traits/ScopedEnum.hpp>
#include <Helena/Traits/Specialization.hpp>

// Types
#include <Helena/Types/Any.hpp>
#include <Helena/Types/BasicLoggersDef.hpp>
#include <Helena/Types/BasicLogger.hpp>
#include <Helena/Types/BenchmarkScoped.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/Delegate.hpp>
#include <Helena/Types/FixedBuffer.hpp>
#include <Helena/Types/Format.hpp>
#include <Helena/Types/Hash.hpp>
#include <Helena/Types/LocationString.hpp>
#include <Helena/Types/Monostate.hpp>
#include <Helena/Types/Mutex.hpp>
#include <Helena/Types/SourceLocation.hpp>
#include <Helena/Types/Spinlock.hpp>
#include <Helena/Types/TaskScheduler.hpp>
#include <Helena/Types/TimeSpan.hpp>
#include <Helena/Types/TSVector.hpp>
#include <Helena/Types/UniqueIndexer.hpp>
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Types/VectorKVAny.hpp>
#include <Helena/Types/VectorUnique.hpp>

// Util
#include <Helena/Util/Cast.hpp>
#include <Helena/Util/ConstexprIf.hpp>
#include <Helena/Util/Format.hpp>
#include <Helena/Util/Length.hpp>
#include <Helena/Util/Sleep.hpp>

#endif // HELENA_HPP
