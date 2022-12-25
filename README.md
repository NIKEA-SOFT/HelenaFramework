![HelenaFramework](https://user-images.githubusercontent.com/57288440/116739956-00ca2580-a9fd-11eb-9c5d-367f21606456.png)

---

# Introduction  

`HelenaFramework` is a header-only, tiny and easy to use library for backend/frontend programming and much more written in **modern C++ 20**  

# Description    
``Background``  
I was looking for solutions for developing game-oriented server applications, but most of the frameworks that I found did not meet my requirements, namely: flexibility, scalability, performance and cross-platform.
A lot of what I found (frameworks) looked pretty good, at least they were frameworks, but I can't say that they were good frameworks, this is because I saw the performance gaps that developers make.
After spending some time getting to know these frameworks, testing them, I came to the realization that if I want to get something really convenient and at the same time fast, then I have to do it myself.
Of course, I wanted to save time, like all of you, because that's why you're here, right? But often we do not think about what we use. How often do you look at the implementation of the libraries that you plan to use in your project? I guess not very often. Most likely, you only look at the documentation or header files, but definitely do not conduct code reviews to evaluate how good the library is for your project.
That is why a popular library does not mean a good library. A good library is STL or boost, but even using them incorrectly can lead to additional performance costs. For me, performance has always been and will always come first.  
Therefore, I developed this framework primarily for myself, but I initially wanted to make it available to the community and therefore its architecture is so flexible. You can use it from scratch or integrate it into an existing project.

Okay, after reading all of this, we might be wondering, what problems does this framework solve?  
To answer this question, you first need to figure out what the framework is, let's figure it out.  
The whole project consists of directories:  
- `Helena`:  
   - `Dependencies` fmt  
   - `Engine` signals, systems, across bounadry  
   - `Platform` cross-platform, macros, assert  
   - `Traits` meta-headers
   - `Types` ready-to-use types: Any, DateTime, RefPtr, StateMachine and other...  
   - `Util` cast's, sleep, format, string length functions  
Helena.hpp - Includes all headers in one file.  

Now that we have a rough idea, we can already guess that the Engine takes on most of the work, and everything else is just ready-made tools for everyday tasks.  
The engine itself gives us architecturally correct code, allowing us to write loosely coupled code between logical parts of the project.  
This really allows you to write more universal code and thus not spend a lot of time refactoring in the future.  
The engine itself does not give us anything to solve specific problems, it may seem strange, why do we need a engine that does not know how to do anything?
In fact, the engine plays a key role, it is the engine that allows you to get type indexing even if you use dll/so and allows you to access systems
from anywhere and moreover, we can simply connect these system classes and get additional logic in our project.  
I didn't specifically use the OOP design so that you could feel how flexible it can be and not costly in terms of performance.  
Why is the architecture exactly like this? Because we can expand it as we like and use only what is required by a particular developer.  
Imagine that I only need a network, in this case, I can just take a ready-made system that is already available in the repository
and just register it inside the framework, it's very simple. I made the engine flexible exactly so that in the future I could supply
any logic without having to change the engine and moreover, I don't need to supply different systems to the developer if he needs only network system.  
I have already implemented a small set of ready-made systems that you can already get and use.
If at the moment there is no system to solve your problem, then just write it and I will add it to the repository
so that other developers can use it and not waste time on it next time.  
It's great, isn't it?
If you have read this to the end, then take a look at the small example below, and also be sure to look in the Examples folder.  

P.S.: You can click on the text `Systems`.
# [Systems](https://github.com/NIKEA-SOFT/HelenaSystems)  

The Systems are classes that implement specific logic.  
Systems can register and throw signals to interact with other systems.  

# Features  

* Header only  
* Cross-platform  
* High performance  
* Scalable and flexible  
* Friendly API  

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
    // Helena::Engine::Context::Initialize();

    // or we can use own context:
    // class MyContext : public Helena::Engine::Context {}
    Helena::Engine::Context::Initialize<MyContext>(/*args for ctor...*/);

    // After initialization, we can register systems or listen for signals
    // You can register and remove systems anywhere
    // You can subscribe, unsubscribe and throw signals anywhere (from callback also)

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}
```