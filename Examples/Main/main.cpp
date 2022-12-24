#include <Helena/Helena.hpp>

class TestSystemA
{
public:
    TestSystemA() {
        // Start listen events
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(&TestSystemA::OnInit);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Config>(&TestSystemA::OnConfig);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Execute>(&TestSystemA::OnExecute);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Tick>(&TestSystemA::OnTick);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Update>(&TestSystemA::OnUpdate);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Render>(&TestSystemA::OnRender);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Finalize>(&TestSystemA::OnFinalize);
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Shutdown>(&TestSystemA::OnShutdown);
    }
    ~TestSystemA() {
        // Unsubscribe from events
        // If listeners are system classes, then such classes can be automatically unsubscribed
        // from events if an object of this class-system has been removed from the Engine.
        // Just remember that this is a good tone
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Init>(&TestSystemA::OnInit);
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Config>(&TestSystemA::OnConfig);
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Execute>(&TestSystemA::OnExecute);
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Tick>(&TestSystemA::OnTick);
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Update>(&TestSystemA::OnUpdate);
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Render>(&TestSystemA::OnRender);
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Finalize>(&TestSystemA::OnFinalize);
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Shutdown>(&TestSystemA::OnShutdown);
    }

    // Note that some events are just empty types,
    // so there is no need to specify them in the callback as an argument.
    // OnInit called once when the system has been registered -> RegisterSystem<T>(...)
    void OnInit() {
        HELENA_MSG_DEBUG("EventInit");
    }

    // OnConfig called once after OnInit event
    // Please note that your listener will be automatically unsubscribed, since this event is called only once
    // this also applies to OnInit and OnExecute
    void OnConfig() {
        HELENA_MSG_DEBUG("EngineConfig");
    }

    // OnExecute called once after OnConfig
    void OnExecute() {
        HELENA_MSG_DEBUG("EngineExecute");
    }

    // This event is called every tick (but after OnExecute)
    // We use the event type as an argument, since it is not an empty type and passes deltaTime
    // Therefore, we cannot ignore it, as it was with OnInit, OnConfig, OnExecute
    void OnTick(const Helena::Events::Engine::Tick event) {
        // // You can uncomment this line, but it is called too often
        //HELENA_MSG_DEBUG("EngineTick: {:.4f}", event.deltaTime);
    }

    // This event is called with a fixed step (but after OnTick)
    // Perhaps you should look into the header file Helena/Engine/Events.hpp
    // It describes all the events of the engine, yes, these are just types
    void OnUpdate(const Helena::Events::Engine::Update event) {
        // You can uncomment this line, but it is called too often
        //HELENA_MSG_DEBUG("EngineUpdate: {:.4f}", event.fixedTime);
    }

    // This event is called every tick
    void OnRender(const Helena::Events::Engine::Render event) {
        // You can uncomment this line, but it is called too often
        //HELENA_MSG_DEBUG("EngineRender: {:.4f}", event.deltaTime);
    }

    // This event is called when the Engine shutdown
    void OnFinalize() {
        HELENA_MSG_DEBUG("EngineFinalize");
    }

    // This event is called after OnFinalize (when Engine shutdown)
    void OnShutdown() {
        HELENA_MSG_DEBUG("EngineShutdown");
    }
};

struct TestSystemB {
    TestSystemB(int age) : age{age} {}
    ~TestSystemB() = default;
    int age;
};

void example_systems()
{
    HELENA_ASSERT(Helena::Engine::HasSystem<TestSystemA>(), "System: {} not registered!", Helena::Traits::NameOf<TestSystemA>{});
    [[maybe_unused]] auto& testSystem = Helena::Engine::GetSystem<TestSystemA>();

    // also we can RegisterSystem in other place
    Helena::Engine::RegisterSystem<TestSystemB>(10);    // register with args...
    HELENA_ASSERT(Helena::Engine::HasSystem<TestSystemB>(), "System: {} not registered!", Helena::Traits::NameOf<TestSystemB>{});

    // also we can use multiple systems by refs
    const auto& [testSystemA, testSystemB] = Helena::Engine::GetSystem<TestSystemA, TestSystemB>();
    HELENA_MSG_NOTICE("TestSystemB age: {}", testSystemB.age);

    // also we can check multiple systems

    // check if all systems are registered
    if(Helena::Engine::HasSystem<TestSystemA, TestSystemB>()) {
        HELENA_MSG_NOTICE("Systems: {} and {} registered!", Helena::Traits::NameOf<TestSystemA>{}, Helena::Traits::NameOf<TestSystemB>{});
    }

    // check if any systems are registered
    if(Helena::Engine::AnySystem<TestSystemA, TestSystemB>()) {
        HELENA_MSG_NOTICE("Systems: {} or {} registered!", Helena::Traits::NameOf<TestSystemA>{}, Helena::Traits::NameOf<TestSystemB>{});
    }

    // OK, now remove systems
    // Don't worry about the cost of Has, Any checks, they are all O(1) and have minimal overhead
    if(Helena::Engine::HasSystem<TestSystemA, TestSystemB>()) {
        Helena::Engine::RemoveSystem<TestSystemA, TestSystemB>();
    }

    // OK, I guess I'll register TestSystemA again for show events
    // Look inside constructor and destructor
    Helena::Engine::RegisterSystem<TestSystemA>();
}

void example_signals()
{
    // so, what are signals in HelenaFramework?
    // The most understandable and simple explanation is an event system
    // that is closely related to class-systems,
    // but you can still use it for ordinary functions
    // that are not related to class-systems or for lambdas without capture.
    // All these restrictions are necessary for performance,
    // I tried to implement them as fast as possible,
    // I think it's obvious that events should notify listeners as quickly as possible and without overhead.
    // And by the way, it's not thread safe but there are reasons for this, which are also related to optimization.

    // Let's look at how to use signals/events
    // The signal can be absolutely any type, but I would recommend struct.
    struct MySignal {};

    // Now let's start listening to this signal
    // Drop arguments if the signal type is std::is_empty_v<MySignal>
    Helena::Engine::SubscribeEvent<MySignal>(+[]() {
        HELENA_MSG_NOTICE("Hello signal: {}", Helena::Traits::NameOf<MySignal>{});
    });

    // Now to notify listeners, it is enough to call
    Helena::Engine::SignalEvent<MySignal>();

    // Other example: signal with args
    struct InfoSignal {
        std::string_view name;
        int age;
    };

    // We cannot drop arguments because InfoSignal is not empty type
    Helena::Engine::SubscribeEvent<InfoSignal>(+[](const InfoSignal ev) {
        HELENA_MSG_NOTICE("Hello signal: {}, name: {}, age: {}",
            Helena::Traits::NameOf<InfoSignal>{}, ev.name, ev.age);
    });

    // Notify
    Helena::Engine::SignalEvent<InfoSignal>("Alex", 30);

    // How to use signals for systems, see the TestSystemA class

    // As for performance Engine, you don't have to worry, in most cases it's O(1)
    // The exception is UnsubscribeEvent, complexity: O(n)
    // But you don't have to worry about it, usually there are very few listeners,
    // and UnsubscribeEvent will be called very rarely or only when execution is completed
}

void example_task_sheduler()
{
    // What is a Task Scheduler?
    // Very often there is a need to perform some kind of operation at a certain time,
    // this type is part of the framework. Let's see how it works.

    // Let's make it static to extend its life
    static Helena::Types::TaskScheduler scheduler;

    // Ok, now let's create some task
    // Example #1
    auto id = 1;        // TaskID (for gamedev: you can use the entity ID to bind a task to an entity)
    auto time = 1000;   // Time in ms, call the task after N ms from the moment of creation
    auto repeat = 1;    // How many times to repeat the task

    // note that callback can accept 'ms' and 'repeat' arguments by reference
    // We can use them to change the time or number of repetitions at any time.
    // This is a more productive method than the Modify method (it requires a hash map search)
    // You can also assign repeat = 0 to complete the task immediately
    scheduler.Create(id, time, repeat, [](std::uint64_t id, std::uint64_t& ms, std::uint32_t& repeat) {
        HELENA_MSG_INFO("Example #1 task id: {}, ms: {}, repeat: {}", id, ms, repeat);
    });

    // Example #2
    id = 2;     // Use a different ID to create a new task

    // We can use lambda capture
    scheduler.Create(id, time, repeat, [value = 55](std::uint64_t id, std::uint64_t& ms, std::uint32_t& repeat) {
        HELENA_MSG_INFO("Example #2 task id: {}, ms: {}, repeat: {}, value: {}", id, ms, repeat, value);
    });

    // Example #3
    id = 3;     // Use a different ID to create a new task

    // We can use lambda capture or args
    scheduler.Create(id, time, repeat, [value = 55](std::uint64_t id, std::uint64_t& ms, std::uint32_t& repeat, std::string msg) {
        HELENA_MSG_INFO("Example #3 task id: {}, ms: {}, repeat: {}, value: {} msg: {}", id, ms, repeat, value, msg);
    }, std::string{"MSG"}); // args


    // Example #4
    // Modify task
    id = 4;     // Use a different ID to create a new task
    repeat = 5; // Change the repeat to 5, we will change it in the callback

    // We can use lambda capture or args
    scheduler.Create(id, time, repeat, [](std::uint64_t id, std::uint64_t& ms, std::uint32_t& repeat) {
        HELENA_MSG_INFO("Example #4 task id: {}, ms: {}, repeat: {}", id, ms, repeat);

        // Before calling current callback, the repeat counter is reduced by 1 in task scheduler
        // In other words, argument repeat say how many repeats are left
        if(repeat) {
            // Change the task parameters
            // Let it be called 1 more time, but after 5 seconds
            ms = 5000; // Change next call time
            repeat = 1; // Change repeat to 1
        }
    });

    // There is another way to modify the task, method: Modify
    // It can be used from anywhere in the code
    // If the task does not exist, then it's no effect behaviour
    // Ok, let's do this
    id = 5;
    time = 1000;
    scheduler.Create(id, time, repeat, [](std::uint64_t id, std::uint64_t& ms, std::uint32_t& repeat) {
        HELENA_MSG_INFO("Example #4 task id: {}, ms: {}, repeat: {}", id, ms, repeat);
    });

    // I think everything is clear here, we just set new values
    // But what does the last flag mean? This is the instant update flag.
    // In other words, when you create a task, it is queued for a certain time,
    // if you pass the Update = true parameter, then this queue will be updated immediately,
    // if you pass the Update = false, the update will occur after the first task call
    // Example (Update = false): first call in 1000 ms and next after 5000 ms
    // Example (Update = true): first and next task call in 5000 ms
    scheduler.Modify(id, 5000, 1, true);

    // Example #5
    // Remove task
    id = 6;
    scheduler.Create(id, time, repeat, [](std::uint64_t id, std::uint64_t& ms, std::uint32_t& repeat) {
        HELENA_MSG_INFO("Example #5 task id: {}, ms: {}, repeat: {}", id, ms, repeat);
        repeat = 0;
    });

    // or from outside use Remove
    scheduler.Remove(id);

    // Note:
    // If you want to modify or remove the task while in callback,
    // then use arguments rather than Modify/Remove, it will be faster.
    // If you need modify or remove from outside, use Modify or Remove

    // Trick for gamedev
    // Since the identifiers are of the uint64_t type, you can store the task ID and the entity ID in it.
    // This can be done using bit shifts

    auto packedID = (100uLL << 8) | (10uLL & 0xFF);  // 100 - EntityID (56 bits), 10 - Task type (8 bits)
    scheduler.Create(packedID, 1000, 1, [](std::uint64_t id, std::uint64_t& ms, std::uint32_t& repeat) {
        const auto entityID = (id >> 8);
        const auto taskType = (id & 0xFF);
        HELENA_MSG_INFO("Task entity id: {}, task id: {}, type: {}, ms: {}, repeat: {}", entityID, id, taskType, ms, repeat);
    });


    // The scheduler cannot work by itself
    // You must call Update so that the scheduler calls events whose time has come
    // Let's use Engine for listen Update event
    Helena::Engine::SubscribeEvent<Helena::Events::Engine::Update>(+[](Helena::Events::Engine::Update) {
        scheduler.Update();
    });

    // Print active task count;
    HELENA_MSG_INFO("Task count: {}", scheduler.Count());

    // Let's summarize the results
    // It's a pretty useful class, isn't it?
    // But what about performance?
    // In fact, I've been racking my brain for a long time how to make it work even faster,
    // but at the same time not lose the following requirements:
    //  1) Fast search by ID
    //  2) Tasks should be sorted by time so that Update checks only the first element
    // So what are the results? Complexity:
    //  Create task: hash map O(1) + vector O(logN)
    //  Search task by ID: O(1)
    //  Remove task by ID: O(1) + O(logN);
    //  Update task (Find): O(1) (it just checks the first element)
    //  Update task (Repeat): O(logN) (when need repeat task after call)
    //  Update task (Remove): O(1) + O(logN) (when need remove task)
}

/* -------------- [Subsystems] ------------- */
#include <Helena/Types/Subsystems.hpp>

// it's subsystem of AnimationManager (just for example subsystems)
struct AnimationSkeleton {};

// it's subsystem of AnimationManager
struct AnimationSpline
    // ModernDesign can be used in Systems (for get `this` from static methods)
    // or in Subsystems for get owner System instance
    : public Helena::Types::ModernDesign<class AnimationManager>
{
    static void StaticFunc();
    void NonStaticFunc();

private:
    int m_Value{};
};

// it's System
class AnimationManager
    : public Helena::Types::ModernDesign<AnimationManager>     // -> Get current system from static methods using CurrentSystem
    , public Helena::Types::SubsystemDesign<AnimationManager>  // -> Create, Any, Has, Remove subsystems
{

public:
    AnimationManager() {
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(&OnInit);  // Subscribe on engine event
    }

private:
    static void OnInit()
    {
        // Get current system AnimationManager
        auto currentSystem = CurrentSystem();

        // Register subsystems in current system (look implementation inside SubsystemDesign)
        currentSystem->RegisterSubsystem<AnimationSpline>();
        currentSystem->RegisterSubsystem<AnimationSkeleton>();

        // Check the exist all of subsystems
        if(currentSystem->HasSubsystem<AnimationSpline, AnimationSkeleton>()) {
            // OK: AnimationSpline and AnimationSkeleton exist
        }

        // Check the exist any of subsystems
        if(currentSystem->AnySubsystem<AnimationSpline, AnimationSkeleton>()) {
            // OK: AnimationSpline or AnimationSkeleton exist
        }

        // Get reference to the some subsystem
        auto& animationSpline = currentSystem->GetSubsystem<AnimationSpline>();
        // or get multiple subsystems
        const auto& [spline, skeleton] = currentSystem->GetSubsystem<AnimationSpline, AnimationSkeleton>();


        // it's example the functional paradigm (SomeFunc it's static method)
        AnimationSpline::StaticFunc();

        // or OOP variant
        spline.NonStaticFunc();

        // Remove systems and destroy objects
        currentSystem->RemoveSubsystem<AnimationSpline, AnimationSkeleton>();
    }
};

void AnimationSpline::StaticFunc() {
    // CurrentSystem() used for get instance of AnimationManager (System) and after GetSubsystem for get "this"
    // Use this technique wisely, for example, in lambda functions you can get a win by omitting the need to capture this
    // this means that you don't need to store lambda anymore.
    auto& self = CurrentSystem()->GetSubsystem<AnimationSpline>();
    const auto value = self.m_Value;

    // for example lambda
    // in oop
    // const auto fnLambda = [this]() {};
    // in modern design paradigm
    // const auto ptr = +[]() {
    //   auto& self = CurrentSystem()->GetSubsystem<AnimationSpline>();
    // }
}

void AnimationSpline::NonStaticFunc() {
    // Nop
}

// Second method of intialization, using own Context
class MyContext : public Helena::Engine::Context
{
private:
    // We can override Main function for some logic
    // for example: RegisterSystems and Signals here or initialize our configs
    // Main function called when you call Helena::Engine::Context::Initialize<MyContext>();
    bool Main() override
    {
        SetTickrate(30.f);      // Same as Helena::Engine::Context::SetTickrate(30.f);

        // Register system's
        Helena::Engine::RegisterSystem<TestSystemA>();
        Helena::Engine::RegisterSystem<AnimationManager>();

        // Or Signals
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(+[]() {
            HELENA_MSG_NOTICE("Hello from MyContext");
        });

        // WARNING: You cannot pass a class method that is not static/raw function from a context or other classes that are not system classes.
        // Signals are mainly intended for class-systems.
        // But they also support lambdas without captures or static methods.
        // Believe me, this is enough to call a method of any class, let me demonstrate
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(+[]() {
            // example #1 for get current context by reference
            auto& context1 = MyContext::template GetInstance<MyContext>();
            // example #2 for get current context by shared_ptr
            auto context2 = MyContext::template Get<MyContext>();
            // I recommend the first option as it is more efficient


            // Now just call our class method
            context1.foo();
        });

        return true;    // true for "no error"
    }

    void foo() {
        HELENA_MSG_NOTICE("Method: foo called");
    }
};


// First method of initialization framework
void Initialization_by_default()
{
    Helena::Engine::Context::Initialize();                  // Initialize Context (Context used in Engine)
    Helena::Engine::Context::SetTickrate(30.f);             // Set Update tickrate

    // Register system's
    Helena::Engine::RegisterSystem<TestSystemA>();
    Helena::Engine::RegisterSystem<AnimationManager>();

    // Or Signals
    Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(+[]() {
        HELENA_MSG_NOTICE("Hello from Initialization_by_default");
    });
}

void Initialization_with_my_Context() {
    // Initialization with own Context, that call Main (if overrided)
    Helena::Engine::Context::Initialize<MyContext>(/* args for constructor */);

    // register system here or in your Context using `bool Main() override`
}

// TODO: test
void test_allocators();

int main(int argc, char** argv)
{
    //Engine started from Initialize method
    Initialization_by_default();            // default initialization
    //Initialization_with_my_Context();     // initialization with own Context

    example_systems();          // ok, here just example how use systems
    example_signals();          // here example with signals
    example_task_sheduler();    // task scheduler example

    test_allocators();

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}

// TODO: Wrapper's for containers in Helena with allocators
void test_allocators()
{
    using String = std::basic_string<char, std::char_traits<char>, Helena::Types::IMemoryAllocator<char>>;
    using Vector = std::vector<String, Helena::Types::IMemoryAllocator<String>>;

    Helena::Types::DefaultAllocator allocator;
    Vector vec_of_string{&allocator};

    vec_of_string.emplace_back("long message for got allocation", &allocator);
    vec_of_string.emplace_back("long message for got allocation", &allocator);
}