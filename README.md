![HelenaFramework](https://user-images.githubusercontent.com/57288440/116739956-00ca2580-a9fd-11eb-9c5d-367f21606456.png)

`HelenaFramework` is a header-only, tiny and easy to use library 
for game backend programming and much more written in **modern C++**.<br/>

---

# Introduction

The HelenaFramework is an architectural pattern used mostly in backend development.

# Features

* User-friendly interface
* High performance
* Modules dll/so with shared memory (across boundary)
* Scalable and flexible
* Cross-platform
* Systems and Events dispatchers
* Header only

## Code Example
```cpp
#include <Common/Helena.hpp>

using namespace Helena;

struct TestSystem 
{
    // Current method called when system initialized (it's system event)
    void OnSystemCreate() {}

    // Current method called after OnSystemCreate (it's system event)
    void OnSystemExecute() {}

    // Called every tick (it's system event)
    void OnSystemTick() {}

    // Called every tickrate (fps) tick (default: 0.016 ms) (it's system event)
    void OnSystemUpdate() {}

    // Called when System destroyed (it's system event)
    void OnSystemDestroy() {}
};

int main(int argc, char** argv)
{
    return Core::Initialize([&]() -> bool 
    {
        // Push args in Core
        Core::SetArgs(argc, argv);

        // Set tickrate
        Core::SetTickrate(60.0);

        // Get args size
        HF_MSG_INFO("Args: {}", Core::GetArgs().size());
        
        // View arguments from vector of std::string_view
        for(const auto& arg : Core::GetArgs()) {
            HF_MSG_INFO("Arg: {}", arg);
        }

        // Systems
        Core::RegisterSystem<TestSystem>();

        return true;
    });
}
```