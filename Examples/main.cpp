﻿#include <Helena/Helena.hpp>
#include <Helena/Concurrency/SPSCQueue.hpp>
#include <Dependencies/moodycamel/readerwriterqueue.h>

// Systems
#include <Helena/Systems/EntityComponent.hpp>
#include <Helena/Systems/ResourceManager.hpp>
#include <Helena/Systems/PropertyManager.hpp>

using namespace Helena;
using namespace Helena::Literals;

struct TestSystem 
{
    // Current method called when system initialized (it's system event)
    void OnSystemCreate() {
        HF_MSG_DEBUG("OnSystemCreate");

        // Test Core events
        Core::RegisterEvent<Events::Initialize,     &TestSystem::OnCoreInitialize>(this);
        Core::RegisterEvent<Events::Finalize,       &TestSystem::OnCoreFinalize>(this);
    }

    // Current method called after OnSystemCreate (it's system event)
    void OnSystemExecute() {
        HF_MSG_DEBUG("OnSystemExecute");

        struct User {
            User() = default;
            User(std::string textA, std::string textB, std::string textC) : m_TextA{std::move(textA)}, m_TextB{std::move(textB)}, m_TextC{std::move(textC)} {
                //HF_MSG_WARN("ctor");
            }
            ~User() {
                //HF_MSG_WARN("dtor");
            }

            User(const User& user) {
                //HF_MSG_WARN("ctor copy");
                m_TextA = user.m_TextA;
                m_TextB = user.m_TextB;
                m_TextC = user.m_TextC;
            }

            User(User&& user) noexcept {
                //HF_MSG_WARN("ctor move");
                std::swap(m_TextA, user.m_TextA);
                std::swap(m_TextB, user.m_TextB);
                std::swap(m_TextC, user.m_TextC);
            }

            User& operator=(const User& user) {
                //HF_MSG_WARN("operator copy");
                m_TextA = user.m_TextA;
                m_TextB = user.m_TextB;
                m_TextC = user.m_TextC;
                return *this;
            }

            User& operator=(User&& user) {
                //HF_MSG_WARN("operator move");
                std::swap(m_TextA, user.m_TextA);
                std::swap(m_TextB, user.m_TextB);
                std::swap(m_TextC, user.m_TextC);
                return *this;
            }

            std::string m_TextA;
            std::string m_TextB;
            std::string m_TextC;
            //std::array<std::size_t, 291> m_Array {};
        };

        constexpr auto size = 1'000'000;
        {
            std::uint32_t counter1 {size};
            std::uint32_t counter2 {size};

            Concurrency::SPSCQueue<User> m_Queue(size);

            for(auto i = size; i != 0; --i) {}

            HF_MSG_WARN("Helena queue start");
            std::thread th1([&m_Queue, &counter1](){
                double timeleft {};
                while(counter1) {
                    if(auto start = Core::GetTimeElapsed(); m_Queue.push("hello world", "hello world", "hello world")) {
                        --counter1;
                        timeleft += Core::GetTimeElapsed() - start;
                    }
                }
                HF_MSG_INFO("Thread 1 finish push elements, timeleft: {}",  timeleft);
            });

            std::thread th2([&m_Queue, &counter2](){
                double timeleft {};
                while(counter2) {
                    if(auto start = Core::GetTimeElapsed(); m_Queue.size()) {
                        auto& instance = m_Queue.front(); m_Queue.pop();
                        --counter2;
                        timeleft += Core::GetTimeElapsed() - start;
                    }
                }
                HF_MSG_INFO("Thread 2 finish pop elements, timeleft: {}",  timeleft);
            });

            if(th1.joinable()) {
                th1.join();
            }

            if(th2.joinable()) {
                th2.join();
            }
        }

        {
            std::uint32_t counter1 {size};
            std::uint32_t counter2 {size};

            moodycamel::ReaderWriterQueue<User> m_Queue(size);

            for(auto i = size; i != 0; --i) {}

            HF_MSG_WARN("Moodycamel queue start");
            std::thread th1([&m_Queue, &counter1](){
                double timeleft {};
                while(counter1) {
                    if(auto start = Core::GetTimeElapsed(); m_Queue.try_enqueue(User{"hello world", "hello world", "hello world"})) {
                        --counter1;
                        timeleft += Core::GetTimeElapsed() - start;
                    }
                }
                HF_MSG_INFO("Thread 1 finish push elements, timeleft: {}",  timeleft);
            });

            std::thread th2([&m_Queue, &counter2](){
                double timeleft {};
                User wtf{"", "", ""};
                while(counter2) {
                    if(auto start = Core::GetTimeElapsed(); m_Queue.try_dequeue(wtf)) {
                        --counter2;
                        timeleft += Core::GetTimeElapsed() - start;
                    }
                }
                HF_MSG_INFO("Thread 2 finish pop elements, timeleft: {}",  timeleft);
            });

            if(th1.joinable()) {
                th1.join();
            }

            if(th2.joinable()) {
                th2.join();
            }
        }

        TestConfigManager();
    }

    // Called when Core initialized
    void OnCoreInitialize(const Events::Initialize& event) {
        HF_MSG_DEBUG("OnCoreInitialize");
    }

    // Called when Core finish
    void OnCoreFinalize(const Events::Finalize& event) {
        HF_MSG_DEBUG("OnCoreFinalize");
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
        HF_MSG_DEBUG("OnSystemDestroy");
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
};


int main(int argc, char** argv)
{
    // Helena example
    return Core::Initialize(
        [argc, argv]() -> bool {
        // Push args in Core
        Core::SetArgs(argc, argv);

        //Core::SetArgs(argc, argv);
        // Set tickrate (30 fps)
        Core::SetTickrate(60.0);

        // Get args size
        HF_MSG_INFO("Args: {}", Core::GetArgs().size());
        
        // View arguments from vector of std::string_view
        for(const auto& arg : Core::GetArgs()) {
            HF_MSG_INFO("Arg: {}", arg);
        }
        
        // test create
        Core::RegisterSystem<Systems::ResourceManager>();   // Хранит объекты любых классов и поддерживает сигналы
        Core::RegisterSystem<Systems::PropertyManager>();   // Хранит любые свойства и поддерживает сигнала
        Core::RegisterSystem<Systems::EntityComponent>();   // Хранит entity, компоненты и поддерживает сигналы
        Core::RegisterSystem<TestSystem>();

        return true;
    });
}
