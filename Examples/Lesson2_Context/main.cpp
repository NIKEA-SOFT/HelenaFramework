#include <Helena/Engine/Engine.hpp>
#include <iostream>

// Lesson 2: Context and how to use your own context in Helena
// A context is a class (singleton) within the Engine that stores complex structures within itself
// that provide access to your systems and signals in O(1) using type indexing.
// The context also provides support for type indexing across boundaries (dll, so plugins).
// When you use Helena::Engine::Initialize it will initialize the default context which has no public fields or methods.
// This decision is due to the fact that I did not want to complicate the API of the framework,
// now all operations are performed only through the Engine class, which contains completely static methods,
// which frees the developer from responsibility for storing the context, engine and other data.
// In Lesson 1 there was an example of how we initialize the default context, of course the method being called doesn't
// say anything about it, but it is.
//
// In the current lesson, I will show you how to create your own context and what it can be useful for.


// Let's declare a class that defines our own context
class MyContext : public Helena::Engine::Context
{
public:
    // *Let's declare the constructors and log them so you can trace the sequence of calls
    MyContext() {
        std::cout << "MyContext ctor called" << std::endl;
    }

    // Note that your context's destructor is called after exiting the application entry point.
    ~MyContext() {
        std::cout << "MyContext dtor called" << std::endl;
    }

public:
    // We can override our own entry point (main)
    // This is not required, but you can use it.
    // This Main does not return the result of the operation.
    // To report an error use Helena::Engine::Shutdown
    void Main() override
    {
        // *Let's leave this log to see the sequence of calls
        std::cout << "Main called" << std::endl;

        // A small example so you can play with it
        bool hasError = true; // change to "true" after checking the result
        if(hasError)
        {
            // Change to "true" for show example with reason message for shutdown
            bool hasReason = true;

            if(!hasReason) {
                // Example #1 | Shutdown without message, this means that no errors occurred.
                Helena::Engine::Shutdown();
            } else {
                // Example #2 | Shutdown with message (reason)
                Helena::Engine::Shutdown("Test shutdown with my test error!");
            }

            return;
        }

        // other operations...
        m_Counter = 10;
    }

public:
    std::size_t m_Counter{};
};

int main(int argc, char** argv)
{
    // Initializing Your Own Context
    // This will create MyContext and call Main if it has been overridden.
    // You can also pass arguments to the constructor.
    Helena::Engine::Initialize<MyContext>(/* args... */);

    // If for some reason Shutdown was called in overridden Main,
    // then the state changed to Shutdown and we can check that
    // NOTE: return is not required here, the Heartbeat can handle Shutdown state
    if(Helena::Engine::GetState() == Helena::Engine::EState::Shutdown) {
        // some logic
    }

    // *Let's leave this log to see the sequence of calls
    std::cout << "Before call Heartbeat" << std::endl;

    // Let me remind you that after calling Initialize, the state of the framework goes to Undefined,
    // and then when the first call to Heartbeat occurs, the state will change to Init.
    // If for some reason Shutdown was called in Main, then the body of this loop will not be executed
    // and the first call Heartbeat will return false.
    while(Helena::Engine::Heartbeat())
    {
        // Get a reference to your context.
        // The method is not thread-safe, but it's actually safe to get since
        // you initialize it once and then it always exists until the application closing.
        auto& ctx = Helena::Engine::GetContext<MyContext>();

        // Shutdown the framework when the counter == 0
        if(ctx.m_Counter == 0) {
            // We do not specify the reason, which means that no errors occurred.
            Helena::Engine::Shutdown();

            // There is no need to write "break", this is wrong!
            // The framework itself must break the loop, as it has signals to notify everyone about the shutdown.
            // But you can use "continue" if you don't want to execute other logic in this loop due to the call to Shutdown.
            continue;
        }

        // Yes, we can continue to use the context after Shutdown,
        // Remember: the context lives the entire life cycle of the program after calling Helena::Engine::Initialize
        std::cout << "Counter: " << ctx.m_Counter-- << std::endl;
    }

    // *Let's leave this log to see the sequence of calls
    std::cout << "After call Heartbeat" << std::endl;

    // About Heartbeat:
    // We can provide own structure for control Sleep and load accumulator
    // Example
    /*
    struct HeartbeatConfig
    {
        // The keyword "Sleep" used for detect our callback
        static constexpr auto Sleep = +[]() {
            Helena::Util::Sleep(1);
        }

        // The keyword "Accumulate" used for detect accumulate value
        // About accumulate read in Heartbeat declaration
        static constexpr auto Accumulate = 5;
    };

    while(Helena::Engine::Heartbeat<HeartbeatConfig>()) {...}
    */


    // - Now we know how to initialize our own context in the framework.
    // Think of a context as a global object that can be accessed from anywhere.
    // That is, you can get your context at any time anywhere in the code,
    // good examples of a context are some kind of `class Application` that stores:
    // argc, argv, directory paths, application name, etc.
    // The context is the only singleton within the framework.


    // WARNING: Calling Engine methods in destructors of static objects is undefined behavior!
    // The Context class is a singleton stored in a static variable, keep this in mind when you
    // decide to try using Engine methods from static variable destructors.
    // Please note that we are talking about the destructor being called by the application itself when the application closes.
    // That is, you can use these methods in destructors if you call these destructors while the application is running.

    return 0;
}