#include <Helena/Engine/Engine.hpp>
#include <iostream>

// Lesson 1: Initialization, Heartbeat and Shutdown

int main(int argc, char** argv)
{
    // Framework initialization
    // This leads to the creation of a context class.
    // Context is a class that stores all the systems, signals, timers and many more.
    // NOTE: There is no method: Finalize, this is not necessary,
    // it will complete itself correctly when the application closes.
    Helena::Engine::Initialize();

    // Framework (non blocking) loop
    // Heartbeat returns false on shutdown or failure
    // This method must be called constantly in a loop to keep the signals working.
    while(Helena::Engine::Heartbeat())
    {
        // other your operations
        // You may be wondering "Why would I insert operations here when I can do it with signals of frameworks?"
        // If you asked this question, then it means that you already know about signals and therefore
        // I will explain it to you right here, and for those who are not yet familiar, about signals in the following lessons.
        // Answer:
        // That's right, you can insert your operations in callbacks of signals or inside Systems, we will also talk about them in other lessons.
        // In fact, everything is much simpler, the framework was developed so that it could be integrated into projects at any stage, that's all.
        std::cout << "Hello from loop" << std::endl;

        // or shutdown framework
        // Note: the Shutdown method can take arguments,
        // but the call by default means the framework exits without any errors
        // This method is thread safe.
        Helena::Engine::Shutdown();

        // here example how shutdown framework with error reason.
        // Note: if you use an overload with arguments, then the framework
        // will notify the reason for shutting down in the console.
        // WARNING: If your application is not a console application,
        // then the messages will not be printed, you must redirect the
        // output of the logs, but I will talk about this in the next lessons
        Helena::Engine::Shutdown("Test error");
        // or with formatting
        Helena::Engine::Shutdown("Shutdown reason: {}", "test error");

        // NOTE:
        // and yes... we can call shutdown multiple time
        // this has no effect if the framework has already been terminated
        // but you must remember that if you called Shutdown with no arguments,
        // then the next call to Shutdown, even with a reason, will not set this message (reason),
        // since the state has already passed to Shutdown



        // When we call Shutdown with a reason, we can get that reason at any time (after shutdown to).
        // If Shutdown was called with no arguments, then this method returns an empty string.
        const auto& reason = Helena::Engine::ShutdownReason();


        // NOTE:
        // After calling Shutdown, the framework will enter the Finalize state,
        // but it will only know about completion when the next call to Heartbeat occurs.
        // You must understand that Shutdown does not exit immediately from process,
        // it only change the state to the Shutdown state, the new state will be processed on the next call to Heartbeat

        // However, we can check the new state after Shutdown immediately
        // This method is thread safe
        // About method Running - this function is the same as calling Helena::Engine::GetState() == Helena::Engine::EState::Init
        if(Helena::Engine::Running()) {
            std::cout << "The framework is still running!" << std::endl;
        } else {
            // This message will be displayed because the call to Shutdown switches the state immediately
            // In order not to confuse you, let me explain:
            // the Shutdown call changes the State inside the kernel, however,
            // the signals (we will talk about in future lessons) will only be thrown after the Heartbeat call
            // The kernel (framework) uses a state machine, so a state change has consequences after the Heartbeat is called again.
            std::cout << "The framework has entered shutdown state! #1" << std::endl;
        }

        // Other method for get state
        // This method is thread safe.
        switch(Helena::Engine::GetState())
        {
            // Undefined state:
            // This state means that the framework has not yet had time to initialize.
            // The main thing here is not to get confused, when you call Initialize,
            // you only set the Undefined state, this state switches to the Init state only
            // after the first call to Heartbeat, in fact, this state is used to initialize
            // some fields inside the kernel, after which the state will change to Init.
            // In short, the Undefined state is only possible in two cases:
            // 1. You called Helena::Engine::Initialize, but the first call to Helena::Engine::Heartbeat has not yet occurred.
            // 2. You called Helena::Engine::Shutdown and after Helena::Engine::Heartbeat is called, the state will change
            // from Shutdown to Undefined again.
            //
            // This approach allows you to restart the loop inside the framework without shutting down the application even
            // if you get an exception (some exceptions that were missed can be caught by the core of the
            // framework to complete the correct operation of applications including signals)
            // Tip: Think of the Undefined state as a car starter, the key is in the lock.
            // This is the best analogy, because in this state you are either going on a trip, or you have completed it :)
            case Helena::Engine::EState::Undefined: {
                std::cout << "The framework is initialized, but state is Undefined" << std::endl;
            } break;


            // Init state:
            // We get this state after the Undefined state.
            // In this state, calls to the Heartbeat method process the main loop: timers, fixed steps, signals
            case Helena::Engine::EState::Init: {
                std::cout << "The framework is running!" << std::endl;
            } break;

            // Shutdown state:
            // This state is set when Helena::Engine::Shutdown is called, or if your application crashes,
            // such as an exception that was not caught, in which case the framework will set the
            // Shutdown state on its own to try to properly shutdown your application, so you don't lose data.
            case Helena::Engine::EState::Shutdown: {
                std::cout << "The framework has entered shutdown state! #2" << std::endl;
            } break;
        }
    }

    // Important Notes:
    // 1) Don't call Helena::Engine::Initialize again, even after calling Helena::Engine::Shutdown,
    // shutdown is just one of the states, not Finalize, the framework will take care of the
    // shutdown when the application closes.
    // 2) You can restart the loop if you want to restart all applications without having to end the
    // process and start it again, however, you must remember that the systems and all signals will be cleared,
    // you will have to register them again. Therefore, it's like restarting the process.


    // Now we know how to initialize the framework.
    // Let's repeat, but without long comments.

    // HasContext used for check the initialization of the framework.
    // About context in Lesson 2
    if(!Helena::Engine::HasContext()) {
        Helena::Engine::Initialize();
    }

    std::size_t сounter{};
    while(Helena::Engine::Heartbeat())
    {
        // Shutdown after 10 iterations
        if(++сounter == 10) {
            // You will see this message in the console after next call Heartbeat, the reason of shutdown.
            Helena::Engine::Shutdown("Shutdown after {} iterations!", сounter);
        }

        std::cout << "Counter = " << сounter << std::endl;
    }

    return 0;
}