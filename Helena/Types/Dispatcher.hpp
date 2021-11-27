#ifndef HELENA_TYPES_DISPATCHER_HPP
#define HELENA_TYPES_DISPATCHER_HPP

//#include <Helena/Debug/Assert.hpp>
//#include <Helena/Types/Hash.hpp>
//#include <Helena/Traits/CVRefPtr.hpp>
//
//#include <vector>
//#include <memory>
//
//namespace Helena::Types 
//{
//    class Dispatcher 
//    {
//        struct IEventPool {
//            virtual ~IEventPool() = default;
//            virtual void Signal() = 0;
//            virtual void Clear() = 0;
//        };
//        
//        template <typename Event>
//        struct EventHandler {
//            static_assert(std::is_same_v<Event, Traits::RemoveCVRefPtr<Event>>, "Event type incorrect");
//            
//            using Callback = std::function<void (Event)>;
//            using Pool = std::vector<Callback>;
//            
//            template <typename Func>
//            void Add(std::uint64_t id, Func&& callback) {
//                HELENA_ASSERT(callback, "Callback is empty!");
//                
//                m_Pool.emplace_back(callback);
//            }
//
//            Pool m_Pool;
//
//
//        };
//
//    };
//}

#endif // HELENA_TYPES_DISPATCHER_HPP
