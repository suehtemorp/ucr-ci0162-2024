#ifndef ECS_HPP
#define ECS_HPP

// Type constraints
#include <concepts>
#include <type_traits>
#include "Utils/Traits.hpp"

// Storage
#include <cstddef>
#include <optional>
#include <tuple>
#include <map>

// Error reporting
#include <exception>

// System interfacing 
#include <functional>
#include <thread>

/// @brief Base type for POD components in an ECS
/// @remark Merely added for disambiguation in templates
/// with respect to other types
class Component {};

/// @brief Base type for service dependencies in an ECS
/// @remark Merely added for disambiguation in templates
/// with respect to other types
class Service {};

/// @brief Restraint required for PODs component in an ECS
template <class T>
concept ComponentType =
    // Enforce POD restraints
    std::is_standard_layout_v<T> &&
    std::is_trivial_v<T> &&
    // Require inheritance from base type (for disambiguation inside templates)
    std::derived_from<T, Component> &&
    // Forbid intersection with services
    !std::derived_from<T, Service>;

/// @brief Restraint required for service dependencies in an ECS
template <class T>
concept ServiceType =
    // Enforce construction assignment restraints
    ((
        std::is_copy_constructible_v<T> &&
        std::is_copy_assignable_v<T>
        ) || (
        std::is_move_constructible_v<T> &&
        std::is_move_assignable_v<T>
    )) &&
    // Enforce destruction contraints
    std::is_destructible_v<T> &&
    // Require inheritance from base type (for disambiguation inside templates)
    std::derived_from<T, Service> &&
    // Forbid intersection with components
    !std::derived_from<T, Component>;

/// @brief Possible entity component systems for given components
/// @tparam ...Components Components to pass onto systems
template <ComponentType... Components>
requires Distinct<Components...>
class ECS {
    public:
        /// @brief Entity component system for given services
        /// @tparam ...Services Services to use accross systems
        template <ServiceType... Services>
        requires Distinct<Services...>
        class WithServices {
            public:
                /// @brief Tuple of components for a given entity
                using Entity = std::tuple<std::optional<Components>...>;

                /// @brief IDs for a given entity
                using EntityID = std::uint64_t;

                /// @brief IDs for a given system
                using SystemID = std::uint64_t;

                /// @brief Service proxy for managing the given ECS  
                class ManagerService : Service {
                    friend class WithServices;

                    private:
                        /// @brief ECS to be defer commands to
                        std::reference_wrapper<WithServices> _managedEcs;

                    protected:
                        /// @brief Construct a proxy for managing a given ECS
                        /// @param managedEcs ECS to manage
                        /// @remark This proxy's lifetime must not exceed the
                        /// managed ECS's
                        ManagerService(WithServices& managedEcs) :
                        _managedEcs(managedEcs) {};

                    public:
                        /// @brief Request that the ECS stop running
                        /// @return True if stop request was honored, 
                        /// false otherwise
                        bool RequestStop() {
                            return _managedEcs.get().RequestStop();
                        }
                };

                /// Make sure the manager service is well-defined
                static_assert(
                    ! ServiceType<ManagerService>, 
                    "Manager service is ill-defined"
                );

                /// Make sure it doesn't collide with other services
                /// (not likely, but who knows)
                static_assert(
                    ! is_any_from<ManagerService, Services...>::value, 
                    "Manager service cannot be a delegate service"
                );

                WithServices() {
                    // Install manager service 
                    std::get<std::optional<ManagerService>>(_services) 
                    = ManagerService(*this);
                }
            
            protected:
                /// @brief Wrapper around a system and its given validator
                class SystemWrapper {
                    friend WithServices;

                    public:
                        using ConsumerWithServices = std::function<
                            void (
                                std::optional<Components>&..., 
                                std::optional<Services>&...,
                                std::optional<ManagerService>&
                            )
                        >;

                        using ConsumerValidator = std::function<
                            bool (
                                const std::optional<Components>&...,
                                const std::optional<Services>&...,
                                const std::optional<ManagerService>&
                            )
                        >;
                    
                    private:
                        /// @brief Wrapper for underlying function that consumes
                        /// components
                        const ConsumerWithServices _consumerWithServices;

                        /// @brief Whether or not the undelying system
                        /// can consume a given entity's components with
                        /// the ECS services
                        const ConsumerValidator _consumerValidator;

                    public:
                        SystemWrapper() = delete;

                        SystemWrapper(
                            const ConsumerWithServices& consumerWithServices,
                            const ConsumerValidator& consumerValidator
                        ) :
                            _consumerValidator(consumerValidator), 
                            _consumerWithServices(consumerWithServices) 
                        {}

                        void AssertValid() const {
                            if (_consumerValidator == nullptr ||
                                _consumerWithServices == nullptr) {
                                throw std::runtime_error("System is ill-informed");
                            }
                        }
                };

                /// @brief Check whether a given system can consume components
                /// on a given entity with the current services
                /// @param system System that could consume components and services
                /// @param entity Entity that could provide components to it
                /// @return True if it can, false otherwise
                bool CanConsumeEntity(const SystemWrapper& system, const Entity& entity) {
                    system.AssertValid();

                    return system._consumerValidator(
                        std::get<std::optional<Components>>(entity)...,
                        std::get<std::optional<Services>>(_services)...,
                        std::get<std::optional<ManagerService>>(_services)
                    );
                }

                /// @brief Refer the consumption of components in a
                /// given entity with given services to a system
                /// @param system System to consume components and services
                /// @param entity Entity to provide components from
                void ConsumeEntity(const SystemWrapper& system, Entity& entity) {
                    if (!CanConsumeEntity(system, entity)) {
                        std::invalid_argument("System cannot consume entity");
                    }

                    system._consumerWithServices(
                        std::get<std::optional<Components>>(entity)...,
                        std::get<std::optional<Services>>(_services)...,
                        std::get<std::optional<ManagerService>>(_services)
                    );
                }

                /// @brief Current entities in existence
                std::map<EntityID, Entity> _entities;

                /// @brief Current systems in existence
                std::map<SystemID, SystemWrapper> _systems;

                /// @brief Services available for use in systems
                std::tuple<std::optional<ManagerService>, std::optional<Services>...> _services;

                /// @brief Thread running the system loops 
                std::jthread _mainLoop;

                /// @brief Next assignable ID for a new entity
                EntityID _nextEntityID = 0;

                /// @brief Next assignable ID for a new system
                SystemID _nextSystemID = 0;

                /// @brief Access a given service in the ECS
                /// @tparam SpecificService Service type to access 
                /// @return Reference to service slot
                template <typename SpecificService>
                requires AnyFrom<SpecificService, Services...>
                inline std::optional<SpecificService>& AccessService() {
                    return std::get<std::optional<SpecificService>>(_services);
                }

                /// @brief Access a given component in a given entity
                /// @tparam SpecificComponent Component type to access
                /// @return Reference to component slot
                template <typename SpecificComponent>
                requires AnyFrom<SpecificComponent, Components...>
                static inline std::optional<SpecificComponent>& 
                AccessComponent(Entity& entity) {
                    return std::get<std::optional<SpecificComponent>>(
                        entity
                    );
                }

                /// @brief Pull an entity reference from storage
                /// @param targetID Valid ID obtained via AddEntity()
                /// @return Iterator to entity in storage
                std::map<EntityID, Entity>::iterator SelectEntity(EntityID targetID) {
                    // Lookup the entity
                    typename std::map<EntityID, Entity>::iterator where = 
                    _entities.find(targetID);

                    // Assert that it is found
                    if (where == _entities.end()) {
                        throw std::invalid_argument("Invalid entity ID");
                    }

                    return where;
                }

                /// @brief Pull a system reference from storage
                /// @param targetID Valid ID obtained via AddSystem()
                /// @return Iterator to system in storage
                std::map<SystemID, SystemWrapper>::iterator SelectSystem(SystemID targetID) {
                    // Lookup the system
                    typename std::map<SystemID, SystemWrapper>::iterator where = 
                    _systems.find(targetID);

                    // Assert that it is found
                    if (where == _systems.end()) {
                        throw std::invalid_argument("Invalid system ID");
                    }

                    return where;
                }

            public:
                /// @brief Add a new entity to the ECS
                /// @tparam ...InitialComponents Component to start with
                /// @param ...components Component values to initialize those
                /// @return A valid ID for further transactions with the entity
                template <typename... InitialComponents>
                requires Distinct<InitialComponents...> &&
                (AnyFrom<InitialComponents, Components...> && ...)
                EntityID AddEntity(InitialComponents&&... components) {
                    // Assign and then increment the latest assignable ID
                    EntityID targetID = _nextEntityID++;

                    // Default-initialize components
                    std::tuple<std::optional<Components>...> tuple;

                    // Fold over passed components and assign each to
                    // its proper field on the tuple
                    (
                        [&] {
                            std::get<
                                std::optional<typeof components>
                            >(tuple) = components;
                        } (),
                    ...);

                    // Insert written tuple onto storage
                    _entities.emplace(targetID, std::move(tuple));

                    // Report ID of inserted entity
                    return targetID;
                }

                /// @brief Remove an existing entity from the ECS
                /// @param targetID Valid ID obtained via AddEntity()
                void RemoveEntity(const EntityID& targetID) {
                    // Remove entity from storage
                    _entities.erase(SelectEntity(targetID));
                }

                /// @brief Install a given component to an existing entity in the ECS
                /// @tparam SpecificComponent Component type to install
                /// @param targetID Valid ID obtained via AddEntity()
                /// @param component Data to assign to component
                template <typename SpecificComponent>
                requires AnyFrom<SpecificComponent, Components...>
                void InstallComponent(EntityID targetID, SpecificComponent&& component) {
                    // Get a reference to the entities' slot 
                    std::optional<SpecificComponent>& slot = 
                    AccessComponent<*SelectEntity(targetID)>();

                    // Sanity check the component is missing
                    if (slot) {
                        throw std::exception("Component already installed");
                    }

                    // Install the component
                    slot = component;
                }

                /// @brief Uninstall a given component from an existing entity in the ECS
                /// @tparam SpecificComponent Component type to uninstall
                /// @param targetID Valid ID obtained via AddEntity()
                template <typename SpecificComponent>
                requires AnyFrom<SpecificComponent, Components...>
                void UninstallComponent(EntityID targetID) {
                    // Get a reference to the entities' slot 
                    std::optional<SpecificComponent>& slot = 
                    AccessComponent<*SelectEntity(targetID)>();

                    // Sanity check the component is present
                    if (!slot) {
                        throw std::exception("Component already uninstalled");
                    }

                    // Uninstall the component
                    slot.reset();
                }

                /// @brief Add a given system to the ECS
                /// @tparam ...SpecificComponents Components qualified-types to consider in system
                /// @tparam ...SpecificServices Services qualified-types to use in system
                /// @param system System to process matching entities
                /// @return A valid ID for further transactions with the system
                template<typename... SpecificComponents, typename... SpecificServices>
                /// Components map 1-to-1 to those in ECS, and must also be references
                requires Distinct<
                    std::remove_cvref_t<SpecificComponents>...
                > && (
                    AnyFrom<
                        std::remove_cvref_t<SpecificComponents>,
                        Components...
                    > 
                    && ...
                ) && (
                    std::is_reference_v<SpecificComponents>
                    && ...
                )
                /// Services must do so as well...
                && Distinct<
                    std::remove_cvref_t<SpecificServices>...
                > && ((
                        AnyFrom<
                            std::remove_cvref_t<SpecificServices>,
                            Services...
                        >
                        // But also consider the manager service
                        || std::is_same_v<
                            std::remove_cvref_t<SpecificServices>, 
                            ManagerService
                        >
                    ) && ...) 
                && (
                    std::is_reference_v<SpecificServices>
                    && ...
                )
                SystemID AddSystem(
                    void (*system) (std::tuple<SpecificComponents...>, 
                    std::tuple<SpecificServices...>)
                ) {
                    // Define a validator for the system
                    typename SystemWrapper::ConsumerValidator validator = [](
                        const std::optional<Components>&... components,
                        const std::optional<Services>&... services,
                        const std::optional<ManagerService>& managerService
                    ) {
                        // Check for applicability across the fold of specific component 
                        // types required by the system
                        if (!(
                            [&] {
                                // On each one, if the corresponding passed component is
                                // available, confirm applicability
                                return std::get<
                                    std::add_lvalue_reference_t<
                                        std::add_const_t<
                                            std::optional<
                                                std::remove_cvref_t<SpecificComponents>
                                            >
                                        >
                                    >
                                >(std::tie(components...)).has_value();
                            } ()
                            && ...)
                        ) {
                            return false;
                        }

                        // Check for applicability across the fold of specific service types 
                        // required by the system
                        if (!(
                            [&]
                            {
                                // On each one, if the corresponding passed service is
                                // available, confirm applicability
                                return std::get<
                                    std::add_lvalue_reference_t<
                                        std::add_const_t<
                                            std::optional<
                                                std::remove_cvref_t<SpecificServices>
                                            >
                                        >
                                    >
                                >(std::tie(managerService, services...)).has_value();
                            } ()
                            && ...)
                        ) {
                            return false;
                        }

                        // If no discard ocurred, accept consumption
                        return true;
                    };

                    // Define a consumer-wrapper for it as well
                    typename SystemWrapper::ConsumerWithServices consumer = 
                    [=](
                        std::optional<Components>&... components,
                        std::optional<Services>&... services,
                        std::optional<ManagerService>& managerService
                    ) {
                        // Forward the parameters to the system call
                        system(
                            std::forward_as_tuple(
                                std::get<
                                    std::add_lvalue_reference_t<
                                        std::optional<
                                            std::remove_cvref_t<SpecificComponents>
                                        >
                                    >
                                >(std::tie(components...)).value()...
                            ),
                            std::forward_as_tuple(
                                std::get<
                                    std::add_lvalue_reference_t<
                                        std::optional<
                                            std::remove_cvref_t<SpecificServices>
                                        >
                                    >
                                >(std::tie(managerService, services...)).value()...
                            )
                        );
                    };

                    // Assign and then increment the latest assignable ID
                    SystemID targetID = _nextSystemID++;
                    _systems.emplace(targetID, SystemWrapper(consumer, validator));

                    // Report ID of inserted entity
                    return targetID;
                }

                /// @brief Remove an existing system from the ECS
                /// @param targetID Valid ID obtained via AddSystem()
                void RemoveSystem(const SystemID& targetID) {
                    // Remove system from storage
                    _systems.erase(SelectSystem(targetID));
                }

                /// @brief Install a given service to the ECS
                /// @tparam SpecificService Service type to install
                /// @param service Unowned service to possess
                template <typename SpecificService>
                requires AnyFrom<SpecificService, Services...>
                void InstallService(SpecificService&& service) {
                    // Get a reference to the service slot
                    std::optional<SpecificService>& slot =
                    AccessService<SpecificService>();

                    // Sanity check that the service is uninstalled
                    if (slot) {
                        throw std::logic_error("Service already installed");
                    }

                    // Install the service
                    slot = std::move(service);
                }

                /// @brief Uninstall a given service from the ECS
                /// @tparam SpecificService Service type to uninstall
                template <typename SpecificService>
                requires AnyFrom<SpecificService, Services...>
                void UninstallService() {
                    // Get a reference to the service slot
                    std::optional<SpecificService>& slot =
                    AccessService<SpecificService>();

                    // Sanity check that the service installed
                    if (!slot) {
                        throw std::exception("Service already uninstalled");
                    }

                    // Uninstall the service
                    slot.reset();
                }
        
                /// @brief Perform one iteration of the update loop
                void Sweep() {
                    for (auto& [systemID, system] : _systems) {
                        for (auto& [entityID, entity] : _entities) {
                            if (CanConsumeEntity(system, entity)) {
                                ConsumeEntity(system, entity);
                            }
                        }
                    }

                    return;
                }

                /// @brief Start running the ECS by dispatching a repeating
                /// cicle of the update loop
                void Start() {
                    _mainLoop = std::jthread(
                        [](std::stop_token stoken, WithServices& ecs) {
                            while (!stoken.stop_requested()) {
                                ecs.Sweep();
                            }
                        },
                        std::ref(*this)
                    );

                    return;
                }

                /// @brief Wait until the ECS has stopped running
                void AwaitStop() {
                    if (!_mainLoop.joinable()) {
                        throw std::logic_error
                        ("ECS is not running in the current thread");
                    }

                    _mainLoop.join();
                }

                /// @brief Request that the ECS stop running
                /// @return True if stop request was honored, 
                /// false otherwise
                bool RequestStop() {
                    return _mainLoop.request_stop();
                }
        };
};

#endif
