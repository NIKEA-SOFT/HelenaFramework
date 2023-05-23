![HelenaFramework](https://user-images.githubusercontent.com/57288440/231351795-b1588eeb-c3ad-4c6a-bb47-76bd68a211f6.png)

---

## Introduction  

`HelenaFramework` is a Universal Modern Framework written in **C++20**  
  
Status: ready to use, but still developing.  

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
- Windows: `MSVC 16.10`, `Clang`, `MSYS2 MinGW/Clang/UCRT`
- Linux: Clang-14 or higher, GCC-13 or higher
- Standard: C++20

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
    > # add_subdirectory(${CMAKE_CURRENT_LIST_DIR)/Dependencies/HelenaFramework)
    > # Open your project dir
    > cd YourProject

    ---------------> [Build: Visual Studio 2022 MSVC/Clang] <------------
    > # Start x86/x64 Native Tools Command Prompt and execute:
    > cmake -B Build -G "Visual Studio 17" -DHELENA_FLAG_EXAMPLES=OFF -DHELENA_FLAG_BIN_DIR=OFF
    > # Open solution in Visual Studion
    ---------------------------------------------------------------------

    ---------------> [Build: MSYS2 MinGW/Clang/UCRT] <-------------------
    > # Start MSYS2 MinGW/Clang/UCRT console
    > # Make sure you have installed all required MSYS2 packages for each compiler (console) version you are using
    > # NOTE:
    > # 1. You can use Debug or Release build type and g++ or clang++ compiler
    > # 2. Other generators besides Ninja are also supported
    > cmake -G Ninja -B Build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DHELENA_FLAG_EXAMPLES=OFF -DHELENA_FLAG_BIN_DIR=OFF
    > # Build
    > cmake --build Build
    ---------------------------------------------------------------------

    --------------------> [Build: Linux GCC/Clang] <---------------------
    > # Start MSYS2 MinGW/Clang/UCRT console
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