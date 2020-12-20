#define CR_HOST

#include <Common/Helena.hpp>
#include <cr/cr.hpp>

void test();

struct my_process : entt::process<my_process, std::uint32_t> 
{
    using delta_type = std::uint32_t;

    my_process(delta_type delay)
        : remaining{delay}
    {}

    void update(delta_type delta, void *) {
        std::cout << "Update called "  << delta << std::endl;
        this->pause();
        if(this->alive() && this->paused()) {
            std::cout << "task pausd, run them" << std::endl;
            this->unpause();
        }
    }

    void init() {
        std::cout << "task init" << std::endl;
    }

    void succeeded() {
        std::cout << "task succeeded" << std::endl;
    }

    void failed() {
        std::cout << "task failed" << std::endl;
    }

    void aborted() {
        std::cout << "task aborted" << std::endl;
    }

private:
    delta_type remaining;
};

struct generatorA {};
struct generatorB {};

struct testA {
    uint32_t val = 10;
};
struct testB {
    uint32_t val = 20;
};
struct testC {
    uint32_t val = 30;
};

/*
template<typename Type>
struct entt::type_seq<Type> {
	[[nodiscard]] static id_type value() ENTT_NOEXCEPT {
		static const entt::id_type value = type_context::instance()->value(entt::type_hash<Type>::value());;
		return value;
	}
};*/

void test()
{
    /*
    using namespace entt::literals;
    entt::scheduler<std::uint32_t> scheduler;
    if(scheduler.empty()) {
        std::cout << "Scheduler is empty" << std::endl;
    }

    scheduler.attach<my_process>(1000u);
    while(true) {
        scheduler.update(100u);
        break;
    }

    // registry test
    {
        entt::registry registry;

        const auto id1 = registry.create();
        registry.emplace<testA>(id1);

        const auto id2 = registry.create();
        registry.emplace<testB>(id2);


    }

    // runtime type indexes
    {
        auto id1 = entt::family<generatorA>::type<testA>;
        auto id2 = entt::family<generatorA>::type<testB>;
        auto id3 = entt::family<generatorA>::type<testC>;

        std::cout << "ID-1 = " << id1 << std::endl;
        std::cout << "ID-2 = " << id2 << std::endl;
        std::cout << "ID-3 = " << id3 << std::endl;
        std::cout << std::endl;

        std::vector<std::shared_ptr<void>> pool;
        pool.emplace_back(std::make_shared<testA>());
        pool.emplace_back(std::make_shared<testB>());
        pool.emplace_back(std::make_shared<testC>());

        auto test_a = std::static_pointer_cast<testA>(pool[id1]);
        auto test_b = std::static_pointer_cast<testB>(pool[id2]);
        auto test_c = std::static_pointer_cast<testC>(pool[id3]);

        std::cout << test_a->val << std::endl;
        std::cout << test_b->val << std::endl;
        std::cout << test_c->val << std::endl;
        std::cout << std::endl;

        // test my templat type_index from line 60
        auto index1 = entt::type_seq<testA>::value();
        std::cout << "template index-1: " << index1 << std::endl;
        auto index2 = entt::type_seq<testB>::value();
        std::cout << "template index-2: " << index2 << std::endl;
        auto index3 = entt::type_seq<testC>::value();
        std::cout << "template index-3: " << index3 << std::endl;
    }
    std::cout << std::endl;

    // compile time type indexed
    {
        using indexes = entt::identifier<testA, testB>;

        std::cout << "ID-1 = " << indexes::type<testA> << std::endl;
        std::cout << "ID-2 = " << indexes::type<testB> << std::endl;
    }
    std::cout << std::endl;

    {
        using indexes = entt::identifier<testA, testB, testC>;
        std::cout << "ID-1 = " << indexes::type<testA> << std::endl;
        std::cout << "ID-2 = " << indexes::type<testB> << std::endl;
        std::cout << "ID-3 = " << indexes::type<testC> << std::endl;
    }
    std::cout << std::endl;

    // hashed string test
    {
        auto load = [](entt::hashed_string::hash_type resource) {
            std::cout << "Hashed string id: " << resource << std::endl;
            return "your resource pointer";
        };

        auto resource = load(entt::hashed_string{"hello world"});
        std::cout << resource << std::endl;
    }
    std::cout << std::endl;

    {
        constexpr auto str = "hello world"_hs;
        std::cout << "Hashed string data: " << str.data() << " id: " << str.value() << std::endl;
    }
    std::cout << std::endl;

    {
        constexpr auto str = L"hello world"_hws;
        std::wcout << "Hashed string data: " << str.data() << " id: " << str.value() << std::endl;
    }   
    std::cout << std::endl;

    // mono state test
    {
        // WARNING: Ѕыть осторожным с типами данных, в примере ниже мы помещаем const char
        // а значит получить его можно только через const char*
        // “акже, данные в monostate хран€тс€ глобально!
        entt::monostate<"hello world"_hs>{} = "hello world";
        const char* data = entt::monostate<"hello world"_hs>{};
        std::cout << "Monostate data: " << data << std::endl;
    }
    std::cout << std::endl;

    // type info test
    {
        constexpr auto id = entt::type_hash<testA>::value();
        constexpr auto name = entt::type_name<testA>::value();
        std::cout << "Typeid: " << id << " name: " << name << std::endl;
    }
    std::cout << std::endl;
    */
    /* ----- [Entity COmponent System] ----- */
    /*
    entt::registry registry;
    struct MyContext {};
    registry.set<MyContext>();
    registry.set<testA>();
    const auto& context = registry.ctx<MyContext>();

    auto& xox = registry.emplace<testB>(registry.create());*/


    /*
    // test entity
    {
        auto entity = registry.create(entt::entity(100000)); // create one entity
        auto wtf = registry.entity(entity);

        std::cout << "WTF: " << entt::to_integral(wtf) << std::endl;
        // test entity version's do and after destroy entity
        {
            auto version = registry.version(entity);
            auto reg_version = registry.current(entity);
            std::cout << "DO Entity: " << entt::to_integral(entity) << ", version: " << version << ", reg_version: " << reg_version << std::endl;
        }

        registry.destroy(entity); // destroy entity

        {
            auto version = registry.version(entity);
            auto reg_version = registry.current(entity);
            std::cout << "AF Entity: " << entt::to_integral(entity) << ", version: " << version << ", reg_version: " << reg_version << std::endl;
        }


        std::vector<entt::entity> entities(50); // create vector with fixed size of 50 elements
        registry.create(entities.begin(), entities.end()); // fill vector with entities
        registry.destroy(entities.begin(), entities.end()); // destroy all entities
    }
    
    // test components
    {
        auto entity = registry.create(entt::entity(100000));

    }

    // test component events
    {
        class Health {
        public:
            Health() : health(100) {}
            ~Health() = default;

        public:
            static void on_construct(entt::registry& registry, entt::entity entity) {
                std::cout << "on_construct called for entity: " << static_cast<uint32_t>(entity) << std::endl;
            }

            static void on_update(entt::registry& registry, entt::entity entity) {
                std::cout << "on_update called for entity: " << static_cast<uint32_t>(entity) << " hp: " << registry.get<Health>(entity).health << std::endl;
            }


            uint32_t health;
        };

        const auto entity = registry.create(entt::entity(1000000));
        entt::observer observer(registry, entt::collector.group<Health>().update<Health>());
        registry.on_construct<Health>().connect<&Health::on_construct>();
        registry.on_update<Health>().connect<&Health::on_update>();
        auto& health = registry.emplace<Health>(entity);
        health.health = 150;

        for(const auto entity: observer) {
            std::cout << "Observer id: " << entt::to_integral(entity) << std::endl;
        }

        registry.patch<Health>(entity, [](auto& hp) {
            hp.health = 100;
        });
        for(const auto entity : observer) {
            std::cout << "Observer idx: " << entt::to_integral(entity) << std::endl;
        }
        observer.clear();   // clear observer entity list
    }

    // test event dispatcher
    {
        
    }

    // test event emmiter
    {
        
        struct my_emmiter : entt::emitter<my_emmiter> 
        {

        };

        
    }

    // test context
    {
        struct MyContext {};
        registry.set<MyContext>();
        const auto& context = registry.ctx<MyContext>();
    }
    */
}

using namespace Helena;
using namespace Helena::Hash::Literals;

//
//struct MyResource {
//
//    std::string m_Name;
//
//private:
//
//};
//
//struct MyResourceLoader : entt::resource_loader<MyResourceLoader, MyResource> 
//{
//    template<typename... Args>
//    auto load(Args&&... args) const {
//        //return MyResource::Create(std::forward<Args>(args)...);
//    }
//};

struct Test {
    Test(std::string_view name) : m_Name{name} {}
    ~Test() = default;

    std::string m_Name;
};

int main(int argc, char** argv)
{
    Core::SetArgs(argc, argv); // storage argc and argv in context
    Core::GetSignal(); // return true if ctrl handler signaled

    ResourceManager::AddResource<Test>("Ivan"_hs, "Hello, Ivan");
    ResourceManager::AddResource<Test>("Vasya"_hs, "Hello, Vasya");
    ResourceManager::AddResource<Test>("Sergey"_hs, "Hello, Sergey");

    ResourceManager::Each<Test>([](auto key, auto& resource) -> void {
        std::cout << "Key: " << key << " It's " << resource->m_Name << std::endl;
    });

    ResourceManager::RemoveStorage<Test>();

    // todo: PluginManager, EventManager, NetManager, LogManager, JobManager, EntityManager
    return 0;
}