![HelenaFramework](https://user-images.githubusercontent.com/57288440/116739956-00ca2580-a9fd-11eb-9c5d-367f21606456.png)

---

## Introduction  

`HelenaFramework` is a header-only, tiny and easy to use library for backend/frontend programming and much more written in **modern C++ 20**  

## Description  
Сollection of types aimed at solving everyday general-purpose tasks.   
The framework itself provides the following features:
- `Context` is the basic data structure that underlies the framework.  
The context is used to store systems, signal and other fields.  
The developer does not have access to the context fields, but you can inherit from it.  
Also, the context serves to support across boundary.  
- `Systems` (classes) are a set of classes following the principle of one class - one logic.  
A similar approach is often practiced in the Gamedev direction, for example,  
you may have a RenderingSystem that will take on the role of rendering-related tasks,  
as well as a NetworkSystem that will deal with the network.  
It is important to understand that the systems I am writing are not tied to the framework.  
If you see a class (system), then most likely the framework itself does not need it to work.  
All systems are separate logical elements, you connect only those that you need to solve your tasks.  
- `Signals` are a simple and convenient mechanism for exchanging information between systems and functions.  
Using signals, you can achieve a good code architecture and reduce dependency in the code.  
- `Support across boundary.`  
In short, it itself is an opportunity to use shared data between an executable process and dynamic libraries.  
The framework itself does not provide the ability to load dynamic libraries (plugins), but it is focused on their support.  
For more information, see the system: PluginManager (plugin support: load, unload, hot reload).  
- `Ready-made sets` of types, traits and util functions.  

Above I have listed the main features thanks to which you get flexibility, scalability and high performance.  
Using these mechanisms wisely, you can achieve the following results: write efficient code with minimal dependencies, 
get rid of spaghetti code, get scalability and flexibility.  

In my free time, I will also develop new systems to solve certain kinds of tasks.  
So that you can save your time and simply connect these systems or plugins with one call.  

Status: Ready to use, but still developing.

## Features  

* Header only
* High Performance
* Scalable and Flexible
* Cross-Platform
* Clean and Friendly API

## Platforms
- Windows
- Linux

Other systems have not been tested or are not supported.

## Compilers
- Windows: MSVC, Clang
- Linux: Clang, GCC

| Compiler | Required flags |
| ------ | ------ |
| GCC | [-fno-gnu-unique](https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html) |
| MSVC | [/Zc:preprocessor](https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor?view=msvc-170) |

Other compilers have not been tested.  

## Code Example
```cpp
#include <Helena/Helena.hpp>

struct MyEvent {
    int value;
};

struct MySystem {  
    MySystem() {
        // Let's listen to the signal that is called when our
        // system is initialized inside the framework
        // All framework signals in header: Helena/Engine/Events.hpp
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init>(&MySystem::OnEventInit);
    }

private:
    // Great, we can hide the signal functions to make the class interface cleaner.
    // Don't worry, it won't stop the signals from notifying us.

    // By the way, we can omit the event argument if the event type is empty.
    // Event type: `Helena::Events::Engine::Init` it's empty type
    // Look in: `Helena/Engine/Events.hpp` header
    void OnEventInit() {
        HELENA_MSG_NOTICE("Hey hello from MySystem");
    }
};

class MyContext : public Helena::Engine::Context {
public:
    // We can create an entry point in our own context
    // It's not required, but you can
    bool Main() override {
        // -- We can register systems and signals right
        // Here is example of how to register listeners
        Helena::Engine::SubscribeEvent<MyEvent>(+[](MyEvent& event){
            // Let's show a message about which event the framework signals us
            HELENA_MSG_NOTICE("Hello event: {}", Helena::Traits::NameOf<MyEvent>{});

                // And let's change value inside event (for example)
                event.value = 100;
        });

        // -- Now let's throw a signal to the listeners
        // First method: we can use the event type and arguments
        // to create the event type inside the method.
        Helena::Engine::SignalEvent<MyEvent>(/*args...*/);
        // Second method: by reference
        // useful if you later want to read some data from the result in-place
        MyEvent event{};
        Helena::Engine::SignalEvent(event);
        // auto value = event.value;
        // value now == 100
        HELENA_MSG_NOTICE("Event value: {}", event.value);

        // Okay, now let's try register own system
        Helena::Engine::RegisterSystem<MySystem>(/*args for constructor...*/);

        return true;  // return true if no error's
    }
};

// Easy to integrate and easy to use anywhere
int main(int argc, char** argv)
{
    // Framework Initialize with default context
    // Helena::Engine::Initialize();

    // or we can use own context:
    // class MyContext : public Helena::Engine::Context {}
    Helena::Engine::Initialize<MyContext>(/*args for ctor...*/);

    // After initialization, we can register systems or listen for signals
    // You can register and remove systems anywhere
    // You can subscribe, unsubscribe and throw signals anywhere (from callback also)

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}
```

## How to build
- Use CMake
- Use any build system (it's header only)