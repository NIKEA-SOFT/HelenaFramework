![HelenaFramework](https://github.com/user-attachments/assets/56c233c8-7cfb-4966-85f0-b34da7d81bd9)

---

## Introduction  

HelenaFramework is a modern and universal framework **C++20**   

## Features  

* Header only
* High Performance
* Scalable and Flexible
* Cross-Platform
* Clean and Friendly API  
***
- ##### Engine:   
  `Systems`, `Components`, `Signals` (events/observer), `Logging`: console, file (async), `Crash Handling` (dump, logs, stacktrace), `Support Across Boundary` (dll,so)   
  `Main Loop Managing (optional)`: tickrate, sleep time, update delta, accumulator of update load.   
  `Main Loop Events (pre/post)`: Init, Config, Execute, Tick, Update, Render, Finalize, Shutdown.   
  
- ##### Platform:   
  `Macros for Asserts (Debug, Runtime)`   
  `Macros for detect Compiler: MSVC, Clang (win, linux), GCC, MINGW`   
  `Macros for detect Platform: Windows, Linux`   
  `Macros for attributes and diagnostics`   
  
- ##### Logging:
  `Built-in log types`: Debug, Info, Notice, Warning, Error, Fatal, Assert, Exception, Memory, Benchmark, Shutdown   
  `Custom logging types and styles`   
  `Console and async file logging`   
  `Runtime mute`   
  `Runtime redirect logging`   
  `Replacing file logger with custom`   
  `Separate logging file for each type or all in one file.`   
  
- ##### Types:
  `Allocators`   
  `Any`   
  `BenchmarkScoped`   
  `CompressedPair`   
  `DateTime`   
  `Delegate`   
  `EncryptedString`   
  `FixedBuffer`   
  `Function`   
  `Hash`   
  `Monostate`   
  `Mutex`   
  `ReferencePointer`   
  `RWLock`   
  `SourceLocation`   
  `Spinlock`   
  `StateMachine`   
  `TaskScheduler`   
  `TimeSpan`   
  
- ##### Traits:   
  `AnyOf`   
  `Arguments`   
  `Cacheline`   
  `Constructible`   
  `FNV1a`   
  `Function`   
  `NameOf`   
  `PowerOf2`   
  `SameAll`   
  `SameAs`   
  `ScopedEnum`   
  
- ##### Util:
  `Cast`   
  `Math`   
  `Process`   
  `String`   
***

Note:   
Using the `Engine` class is optional.   
This means that to use logging, types, traits, util you don't have to initialize the Engine or use it.  

## Platforms
- Windows
- Linux

Other systems have not been tested or are not supported.

## Compilers
- Windows: `MSVC 16.10`, `Clang`, `MSYS2 MinGW/UCRT`
- Linux: Clang-15 or higher, GCC-13 or higher
- Standard: C++20  
  
| Compiler | Required flags |
| ------ | ------ |
| MSVC | [/Zc:preprocessor](https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor?view=msvc-170) |
  
WARNING: Clang libc++ is temporarily unsupported, waiting for chrono support for std::formatter  
Other compilers have not been tested.  

## Code Example
```cpp
#include <Helena/Helena.hpp>

struct MyEvent {
    int value{};
};

struct MySystem {  
    MySystem() {
        // Subscribe on event
        // All framework events in header: Helena/Engine/Events.hpp
        Helena::Engine::SubscribeEvent<Helena::Events::Engine::Init, &MySystem::OnEventInit>(this);
    }

    ~MySystem() {
        Helena::Engine::UnsubscribeEvent<Helena::Events::Engine::Init, &MySystem::OnEventInit>(this);
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
    // You can create your own entry point
    void Main() override {
        // Register systems and signals
        // Here is example of how to register listeners
        Helena::Engine::SubscribeEvent<MyEvent, [](MyEvent& event){
            // Let's show a message about which event the framework signals us
            HELENA_MSG_NOTICE("Hello event: {}", Helena::Traits::NameOf<MyEvent>{});

                // And let's change value inside event (for example)
                event.value = 100;
        }>();

        // Now let's throw a signal to the listeners
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
        // or use deferred signals, they will be called in the next tick
        Helena::Engine::EnqueueSignal<MyEvent>(/*args...*/);

        // Okay, now let's try register own system
        Helena::Engine::RegisterSystem<MySystem>(/*args for constructor...*/);

        // The System Instance can be accessed from anywhere in the code
        // Access from dynamic libraries is also supported
        if(!Helena::Engine::HasSystem<MySystem>()) {
            return;
        }

        // Accessing elements in O(1) without a hashmap
        auto& mySystem = Helena::Engine::GetSystem<MySystem>();
    }
};

// Easy to integrate and easy to use anywhere
// Initialization and non blocking heartbeat (loop)
int main(int argc, char** argv)
{
    // Framework Initialize with default context
    // Helena::Engine::Initialize();

    // or we can use own context:
    Helena::Engine::Initialize<MyContext>(/*args for ctor...*/);

    // Engine loop
    while(Helena::Engine::Heartbeat()) {}

    return 0;
}
```

## How to build
---
#### CMake

| Flags | Default | Description |
| ------ | ------ | ------ |
| HELENA_FLAG_TEST | OFF | Tests, temporarily unavailable |
| HELENA_FLAG_EXAMPLES | ON | Examples of using |
| HELENA_FLAG_VIEW_HELENA | OFF | Display Helena header files in your project |
| HELENA_FLAG_BIN_DIR | ON | Changes output directories for binaries to `${CMAKE_SOURCE_DIR}/Bin` |  

> `Note:` recommend disable `HELENA_FLAG_EXAMPLES` and `HELENA_FLAG_BIN_DIR` if you want to use the library in your project.  
If you want to display header files, I recommend also calling `HELENA_SOURCE_PRETTY()` in your CMakeLists,  
this will group the files so they don't get scattered around your project.  
Usage flags in cmake: `-DHELENA_FLAG_...=ON/OFF`   

| Util Functions | Description |
| ------ | ------ |
| HELENA_SOURCE_PRETTY | Group the framework header files in the "Helena" catalog, has effect only when flag `HELENA_FLAG_VIEW_HELENA` is `ON` |
| HELENA_SOURCE_GROUP | Group your own files |
| HELENA_SOURCE_GLOB | Recursively search for files with extensions in a directory |  

> `Note:` see examples of usage in Examples! 

```sh
    > # Open yout project dependencies dir
    > cd YourProject/Dependencies
    > git pull https://github.com/NIKEA-SOFT/HelenaFramework.git
    > # Edit your project CMakeLists.txt and add:
    > add_subdirectory(${CMAKE_CURRENT_LIST_DIR)/Dependencies/HelenaFramework)
    > target_link_libraries(${YOUR_PROJECT} PRIVATE Helena::Helena)
    > # Open your project dir
    > cd YourProject

    ---------------> [Build: Visual Studio 2022 MSVC/Clang] <------------
    > # Start x86/x64 Native Tools Command Prompt and execute:
    > cmake -B Build -G "Visual Studio 17" -DHELENA_FLAG_EXAMPLES=OFF -DHELENA_FLAG_BIN_DIR=OFF
    > # Open solution in Visual Studion
    ---------------------------------------------------------------------

    ---------------> [Build: MSYS2 MinGW/Clang/UCRT] <-------------------
    > # Start MSYS2 MinGW/UCRT console
    > # Make sure you have installed all required MSYS2 packages for each compiler (console) version you are using
    > # NOTE:
    > # 1. You can use Debug or Release build type
    > # 2. Other generators besides Ninja are also supported
    > cmake -G Ninja -B Build -DCMAKE_BUILD_TYPE=Debug -DHELENA_FLAG_EXAMPLES=OFF -DHELENA_FLAG_BIN_DIR=OFF
    > # Build
    > cmake --build Build
    ---------------------------------------------------------------------

    --------------------> [Build: Linux GCC/Clang] <---------------------
    > # Install Clang-15 and GCC-13 or higher
    > # WARNING: Clang libc++ temporarily unsupported
    > # NOTE:
    > # 1. You can use Debug or Release build type and g++ or clang++ compiler
    > # 2. Other generators besides Ninja are also supported
    > cmake -G Ninja -B Build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DHELENA_FLAG_EXAMPLES=OFF -DHELENA_FLAG_BIN_DIR=OFF
    > # Build
    > cmake --build Build
    ---------------------------------------------------------------------
```
  
#### No build systems
```
    Installing:
    1. Download ZIP archive from repository
    2. Open your "Project/Dependencies" directory
    3. Copy "Helena" dir from ZIP to "Project/Dependencies" dir
    4. Add "Project/Dependencies" into "include directories" of your project!
    Done!

    WARNING: Make sure you remember to add the flags listed in the "Compilers" section of the README
```