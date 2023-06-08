#include <Helena/Engine/Engine.hpp>
#include <iostream>

// Lesson 2: Context and how to use your own context in Helena
// A context is a class within the Engine that stores complex structures within itself
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
            bool hasReason = false;

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
    // This will create MyContext and then call Main if it has been overridden.
    // You can also pass arguments to the constructor.
    // The call sequence is as follows:
    // Initialize -> Create Context object -> Call Main (if overrided)
    Helena::Engine::Initialize<MyContext>(/* args... */);

    // You may notice that the Initialize method returns void and
    // wonder "How do you know if there was a failure during initialization?"
    // It's really simple, use Helena::Engine::GetState()
    // In Lesson 1, we already got acquainted with this function.
    // Let me remind you that the method is thread-safe.
    if(Helena::Engine::GetState() == Helena::Engine::EState::Shutdown) {
        // We can also get the reason for the failure.
        // NOTE: I'm not sure if there is a need for this,
        // since it is possible to override the output for any type of log message.
        // I will talk about this in the future...
        const auto& reason = Helena::Engine::ShutdownReason();
        if(reason.empty()) {
            std::cout << "Context initialize failed, shutdown without error (reason)" << std::endl;
        } else {
            std::cout << reason << std::endl;
        }

        // Important note:
        // In fact, Heartbeat will process the Shutdown itself and you won't get into the
        // body of the loop anyway, so you don't need to return and check the state on Shutdown,
        // only use this if your code needs it.
        // In addition, Heartbeat will show you the reason for the shutdown in console
    } else {
        std::cout << "Context initialized successfully!" << std::endl;
    }

    std::cout << "Before call Heartbeat" << std::endl;

    // I didn't mention it in the Lesson 1, but the Heartbeat method is also capable of taking arguments.
    // Please read about arguments in method declaration, where it is described in more detail.
    const auto sleepMS = 1; // 1 ms
    const auto accumulator = 5; // High load update regulator (delta accumulator)
    while(Helena::Engine::Heartbeat(sleepMS, accumulator))
    {
        // Get a reference to your context
        // The method is not thread-safe, but it's actually safe to get since
        // you initialize it once and then it always exists until the application exits.
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

    // - Now we know how to initialize our own context in the framework.

    return 0;
}