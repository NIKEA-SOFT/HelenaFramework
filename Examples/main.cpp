//#include <Helena/Helena.hpp>
//#include <Helena/Concurrency/SPSCQueue.hpp>
//#include <Helena/Concurrency/ThreadPool.hpp>
//#include <Helena/Concurrency/ParallelPool.hpp>
//#include <Helena/Concurrency/SpecificQueue.hpp>

// Systems
//#include <Helena/Systems/EntityComponent.hpp>
//#include <Helena/Systems/ResourceManager.hpp>
//#include <Helena/Systems/PropertyManager.hpp>

//#include <Helena/Debug/Assert.hpp>
//#include <Helena/Core/Log.hpp>

#include <iostream>

/*
struct TestSystem 
{
    // Current method called when system initialized (it's system event)
    void OnSystemCreate() {
        //HF_MSG_DEBUG("OnSystemCreate");

        // Test Core events
        Core::RegisterEvent<Events::Initialize,     &TestSystem::OnCoreInitialize>(this);
        Core::RegisterEvent<Events::Finalize,       &TestSystem::OnCoreFinalize>(this);
    }

    // Current method called after OnSystemCreate (it's system event)
    void OnSystemExecute() {
        //HF_MSG_DEBUG("OnSystemExecute");

        //TestConfigManager();

        //const std::size_t COUNT = 10'000'000

        //Util::Sleep(10);
    }

    // Called when Core initialized
    void OnCoreInitialize(const Events::Initialize& event) {
        //HF_MSG_DEBUG("OnCoreInitialize");
    }

    // Called when Core finish
    void OnCoreFinalize(const Events::Finalize& event) {
        //HF_MSG_DEBUG("OnCoreFinalize");
    }

    // Called every tick (it's system event)
    void OnSystemTick() {
        //HF_MSG_DEBUG("OnSystemTick");
    }

    // Called every tickrate (fps) tick (default: 0.016 ms) (it's system event)
    void OnSystemUpdate() {
        //HF_MSG_DEBUG("OnSystemUpdate, delta: {:.4f}", Core::GetTimeDelta());
    }

    // Called when System destroyed (it's system event)
    void OnSystemDestroy() {
        //HF_MSG_DEBUG("OnSystemDestroy");
    }

    void TestConfigManager() {
        // Get ConfigManager system
        auto& resources = Core::GetSystem<Systems::ResourceManager>();

        // Test Resources

        // test structure
        struct Info {
            std::string name;
            std::string url;
            float version;
        };

        // test structure
        struct Book {
            std::string name;
            std::string author;
        };

        HF_MSG_DEBUG("Info size: {}, book size: {}", sizeof(Info), sizeof(Book));

        // Initialize and store the property inside config manager container
        resources.Create<Info>("Helena Framework", "github.com/nikea-soft", 0.1f);
        resources.Create<Book>("Effective Modern C++", "Scott Meyers");

        // Check on exist
        if(resources.Has<Info, Book>()) {
            HF_MSG_DEBUG("Resource Data exist!");
        }

        // Check the availability of one of the Resources
        if(resources.Any<Info, Book>()) {
            HF_MSG_DEBUG("Any of the Resource exist");
        }

        // deduction guide support
        [[maybe_unused]] const auto& [info, book] = resources.Get<Info, Book>();

        // or get one of Resource
        [[maybe_unused]] const auto& info_ = resources.Get<Info>();

        //HF_MSG_INFO("Test Resource, info name: {}, url: {}, version: {:.2f}", info.name, info.url, info.version);
        //HF_MSG_INFO("Test Resource, book name: {}, author: {}", book.name, book.author);

        resources.Remove<Info, Book>();

        // Test Properties
        auto& properties = Core::GetSystem<Systems::PropertyManager>();

        // Using of properties
        using Version = Systems::PropertyManager::Property<"Version"_hs, std::string>;
        using Speed = Systems::PropertyManager::Property<"Speed"_hs, float>;
        using Velocity = Systems::PropertyManager::Property<"Velocity"_hs, float>;

        // Initialize and store the property inside config manager container
        properties.Create<Version>("Alpha 0.1");
        properties.Create<Speed>(100.f);
        properties.Create<Velocity>(200.f);

        // Check on exist
        if(properties.Has<Version, Speed, Velocity>()) {
            HF_MSG_DEBUG("Properties exist!");
        }

        // Check the availability of one of the properties
        if(properties.Any<Version, Speed, Velocity>()) {
            HF_MSG_DEBUG("Any of the property exist");
        }

        // deduction guide support
        [[maybe_unused]] const auto& [version, speed, velocity] = properties.Get<Version, Speed, Velocity>();

        // or get one of property
        [[maybe_unused]] const auto& version_ = properties.Get<Version>();

        HF_MSG_INFO("Test Property, version: {}, speed: {:.2f}, velocity: {:.2f}", version, speed, velocity);

        // remove property
        properties.Remove<Version>();           // Remove one property
        properties.Remove<Speed, Velocity>();   // Remove multiple property
    }
};*/

#include <Helena/Engine/Engine.hpp>
#include <Helena/Types/Hash.hpp>
#include <Helena/Types/FixedString.hpp>
#include <Helena/Types/FixedBuffer.hpp>
#include <Helena/Types/Format.hpp>
#include <Helena/Types/TimeSpan.hpp>
#include <Helena/Types/DateTime.hpp>
#include <Helena/Types/VectorUnique.hpp>
#include <Helena/Types/VectorAny.hpp>
#include <Helena/Util/Format.hpp>
#include <Helena/Platform/Windows/Windows.hpp>
#include <Helena/Engine/Log.hpp>
#include <Helena/Memory/CacheAllocator.hpp>
#include <Helena/Traits/CVRefPtr.hpp>


using namespace Helena;

class MyContext : public Core::Context
{
public:
    MyContext(int val) : value(val) {}
    ~MyContext() = default;

    int value = 100;
};

// Это евент
struct MyEvent {
    Types::FixedBuffer<32> m_Name;
    int m_Age;
};

// Это класс системы
struct MySystem {
    MySystem(int value) : m_Value(value) {}
    ~MySystem() = default;

    int m_Value;

    void CallbackEvent(MyEvent& data) {
        HELENA_MSG_WARNING("Callback called, your name: {}, age: {}", data.m_Name, data.m_Age);
    }
};

int main(int argc, char** argv)
{
    const auto ctx = Core::Context::Initialize<MyContext>(500);

    ctx->SetAppName("Test");
    ctx->SetTickrate(120);
    ctx->m_CallbackInit.Connect<+[]() {
        HELENA_MSG_INFO("Инициализация ядра!");
        Engine::RegisterSystem<MySystem>(100500);
        Engine::SubscribeEvent<MyEvent>(&MySystem::CallbackEvent); // тут подписались
    }>();

    ctx->m_CallbackTick.Connect<+[]() {
        HELENA_MSG_INFO("Tick");
        Engine::SignalEvent<MyEvent>("Oleg", 22); // тут кинули сигнал
        Engine::RemoveEvent<MyEvent>(&MySystem::CallbackEvent);
    }>();

    ctx->m_CallbackUpdate.Connect<+[]() {
        //HELENA_MSG_INFO("Update");
        Engine::Shutdown();
    }>();

    ctx->m_CallbackShutdown.Connect<+[]() {
        if(Engine::HasSystem<MySystem>()) {
            const auto& system = Engine::GetSystem<MySystem>();
            HELENA_MSG_INFO("Your system name: {}, value: {}", Traits::NameOf<Traits::RemoveCVRefPtr<decltype(system)>>::value, system.m_Value);

            Engine::RemoveSystem<MySystem>();
            if(!Engine::HasSystem<MySystem>()) {
                HELENA_MSG_INFO("Your system removed successfully!");
            }
        }

    }>();


    while(Engine::Heartbeat()) {}

    return 0;
}
