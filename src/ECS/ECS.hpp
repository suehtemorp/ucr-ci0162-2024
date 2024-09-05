#pragma once

// ECS system & component
#include "ECS/ECS_Core.hpp"

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

                /// @brief IDs for a given service action
                using ServiceActionID = std::uint64_t;

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
                /// TODO: Refactor Wrappers onto common class?

                /// @brief Wrapper around a system and its given validator
                class SystemWrapper {
                    friend WithServices;

                    public:
                        using Consumer = std::function<
                            void (
                                std::optional<Components>&..., 
                                std::optional<Services>&...,
                                std::optional<ManagerService>&
                            )
                        >;

                        using ComponentValidator = std::function<
                            bool (const std::optional<Components>&...)
                        >;

                        using ServiceValidator = std::function<
                            bool (
                                const std::optional<Services>&...,
                                const std::optional<ManagerService>&
                            )
                        >;
                    
                    private:
                        /// @brief Wrapper for underlying function that consumes
                        /// components and systems
                        const Consumer _consumer;

                        /// @brief Whether or not the undelying system
                        /// could consume a given entity's components
                        const ComponentValidator _componentValidator;

                        /// @brief Whether or not the undelying system
                        /// could consume a given set of services
                        const ServiceValidator _serviceValidator;

                    public:
                        SystemWrapper() = delete;

                        SystemWrapper(
                            const Consumer& consumerWithServices,
                            const ComponentValidator& componentValidator,
                            const ServiceValidator& serviceValidator
                        ) :
                            _consumer(consumerWithServices), 
                            _componentValidator(componentValidator), 
                            _serviceValidator(serviceValidator)
                        {}

                        /// @brief Assert that this wrapper is not invalid or
                        /// ill-informed
                        void AssertValid() const {
                            if (_consumer == nullptr ||
                                _componentValidator == nullptr ||
                                _serviceValidator == nullptr
                            ) {
                                throw std::runtime_error
                                ("System is ill-informed");
                            }
                        }
                
                        /// @brief Check whether the underlying system can consume 
                        /// components on a given entity
                        /// @param entity Possible provided components
                        /// @return True if it can, false otherwise
                        bool CanConsumeComponents(const Entity& entity) const {
                            this->AssertValid();

                            return this->_componentValidator(
                                std::get<std::optional<Components>>(entity)...
                            );
                        }

                        /// @brief Check whether the underlying system can consume
                        /// some optionally-provided services
                        /// @param services Possibly provided services
                        /// @return True if it can, false otherwise
                        bool CanConsumeServices(
                            const std::tuple<
                                std::optional<ManagerService>, 
                                std::optional<Services>...
                            >& services
                        ) const {
                            this->AssertValid();

                            return this->_serviceValidator(
                                std::get<std::optional<Services>>(services)...,
                                std::get<std::optional<ManagerService>>(services)
                            );
                        }

                        /// @brief Forward the consumption of components in a
                        /// given entity with given services to the underlying system
                        /// @param services Services that may be consumed 
                        /// @param entity Entity to provide components from
                        void ConsumeEntity(
                            Entity& entity,
                            std::tuple<std::optional<ManagerService>, std::optional<Services>...>& services
                        ) {
                            if (! this->CanConsumeComponents(entity)) {
                                std::invalid_argument("System cannot consume entity");
                            }

                            if (! this->CanConsumeServices(services)) {
                                std::invalid_argument("System cannot consume services");
                            }

                            this->_consumer(
                                std::get<std::optional<Components>>(entity)...,
                                std::get<std::optional<Services>>(services)...,
                                std::get<std::optional<ManagerService>>(services)
                            );
                        }
                };

                /// @brief Wrapper around an invokable action taken on some services
                class ServiceActionWrapper {
                    friend WithServices;

                    public:
                        using Consumer = std::function<
                            void (
                                std::optional<Services>&...,
                                std::optional<ManagerService>&
                            )
                        >;

                        using Validator = std::function<
                            bool (
                                const std::optional<Services>&...,
                                const std::optional<ManagerService>&
                            )
                        >;
                    
                    private:
                        /// @brief Wrapper for underlying function that 
                        /// consumes services
                        const Consumer _consumer;

                        /// @brief Whether or not the undelying service
                        /// action could consume a given set of services
                        const Validator _validator;

                    public:
                        ServiceActionWrapper() = delete;

                        ServiceActionWrapper(
                            const Consumer& consumer,
                            const Validator& validator
                        ) :
                            _consumer(consumer), 
                            _validator(validator)
                        {}

                        /// @brief Assert that this wrapper is not invalid or
                        /// ill-informed
                        void AssertValid() const {
                            if (_consumer == nullptr ||
                                _validator == nullptr
                            ) {
                                throw std::runtime_error
                                ("Service action is ill-informed");
                            }
                        }

                        /// @brief Check whether the underlying system can consume
                        /// some optionally-provided services
                        /// @param services Possibly provided services
                        /// @return True if it can, false otherwise
                        bool CanConsumeServices(
                            const std::tuple<
                                std::optional<ManagerService>, 
                                std::optional<Services>...
                            >& services
                        ) const {
                            this->AssertValid();

                            return this->_validator(
                                std::get<std::optional<Services>>(services)...,
                                std::get<std::optional<ManagerService>>(services)
                            );
                        }

                        /// @brief Forward the consumption of the given services to
                        /// the underlying system
                        /// @param services Services that may be consumed
                        void ConsumeServices(
                            std::tuple<
                                std::optional<ManagerService>, 
                                std::optional<Services>...
                            >& services
                        ) {
                            if (! this->CanConsumeServices(services)) {
                                std::invalid_argument
                                ("Service action cannot consume services");
                            }

                            this->_consumer(
                                std::get<std::optional<Services>>(services)...,
                                std::get<std::optional<ManagerService>>(services)
                            );
                        }
                };

                /// @brief Current entities in existence
                std::map<EntityID, Entity> _entities;

                /// @brief Current systems in existence
                std::map<SystemID, SystemWrapper> _systems;

                /// @brief Current service actions in existence
                std::map<ServiceActionID, ServiceActionWrapper> _serviceActions;

                /// @brief Services available for use in systems
                std::tuple<std::optional<ManagerService>, std::optional<Services>...> _services;

                /// @brief Dispatched thread running the system loops 
                std::jthread _mainLoop;

                /// @brief Whether or not the current thread should keep
                /// running the system loops
                bool _localContinue = false;

                /// @brief Next assignable ID for a new entity
                EntityID _nextEntityID = 0;

                /// @brief Next assignable ID for a new system
                SystemID _nextSystemID = 0;

                /// @brief Next assignable ID for a new service action
                ServiceActionID _nextServiceActionID = 0;

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

                /// @brief Pull a service action reference from storage
                /// @param targetID Valid ID obtained via AddServiceAction()
                /// @return Iterator to service action in storage
                std::map<ServiceActionID, ServiceActionWrapper>::iterator 
                SelectServiceAction(ServiceActionWrapper targetID) {
                    // Lookup the service action
                    typename std::map<ServiceActionID, ServiceActionWrapper>::iterator 
                    where = _serviceActions.find(targetID);

                    // Assert that it is found
                    if (where == _serviceActions.end()) {
                        throw std::invalid_argument("Invalid service action ID");
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
                    AccessComponent<SpecificComponent>(*SelectEntity(targetID));

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
                    AccessComponent<SpecificComponent>(*SelectEntity(targetID));

                    // Sanity check the component is present
                    if (!slot) {
                        throw std::exception("Component already uninstalled");
                    }

                    // Uninstall the component
                    slot.reset();
                }

                /// TODO: Lots of boilerplate. Refactor into generic call?

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
                    // Define validators for the system...
                    
                    // - On components
                    typename SystemWrapper::ComponentValidator componentValidator = [](
                        const std::optional<Components>&... components
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

                        // If no discard ocurred, accept consumption
                        return true;
                    };

                    // - On services
                    typename SystemWrapper::ServiceValidator serviceValidator = [](
                        const std::optional<Services>&... services,
                        const std::optional<ManagerService>& managerService
                    ) {
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
                    typename SystemWrapper::Consumer consumer = 
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
                    _systems.emplace(targetID, 
                        SystemWrapper(
                            consumer, componentValidator, serviceValidator
                    ));

                    // Report ID of inserted entity
                    return targetID;
                }

                /// @brief Remove an existing system from the ECS
                /// @param targetID Valid ID obtained via AddSystem()
                void RemoveSystem(const SystemID& targetID) {
                    // Remove system from storage
                    _systems.erase(SelectSystem(targetID));
                }

                /// @brief Add a given service action to the ECS
                /// @tparam ...SpecificServices Services qualified-types to use in the
                /// service action
                /// @param serviceAction Service action to process matching services
                /// @return A valid ID for further transactions with the system
                template<typename... SpecificServices>
                /// Services map 1-to-1 to those in ECS, and must also be references
                requires Distinct<
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
                ServiceActionID AddServiceAction(
                    void (*serviceAction) (SpecificServices...)
                ) {
                    // Define a validator for the service action based on
                    // its services
                    typename ServiceActionWrapper::Validator validator = [](
                        const std::optional<Services>&... services,
                        const std::optional<ManagerService>& managerService
                    ) {
                        // Check for applicability across the fold of specific service types 
                        // required by the service action
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
                    typename ServiceActionWrapper::Consumer consumer = 
                    [=](
                        std::optional<Services>&... services,
                        std::optional<ManagerService>& managerService
                    ) {
                        // Forward the parameters to the system call
                        serviceAction(
                            std::get<
                                std::add_lvalue_reference_t<
                                    std::optional<
                                        std::remove_cvref_t<SpecificServices>
                                    >
                                >
                            >(std::tie(managerService, services...)).value()...
                        );
                    };

                    // Assign and then increment the latest assignable ID
                    ServiceActionID targetID = _nextServiceActionID++;
                    _serviceActions.emplace(targetID, ServiceActionWrapper(consumer, validator));

                    // Report ID of inserted service action
                    return targetID;
                }

                /// @brief Remove an existing service action from the ECS
                /// @param targetID Valid ID obtained via AddServiceAction()
                void RemoveServiceAction(const ServiceActionID& targetID) {
                    // Remove service action from storage
                    _serviceActions.erase(SelectServiceAction(targetID));
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
                    /// Sweep over service actions...
                    for (auto& [serviceActionID, serviceAction] : _serviceActions) {
                        if (serviceAction.CanConsumeServices(_services)) {
                            serviceAction.ConsumeServices(_services);
                        }
                    }

                    /// ... And ECS matchings independently
                    for (auto& [systemID, system] : _systems) {
                        /// WARN: For now its a given that within a given
                        /// ECS sweep, no services can be uninstalled.
                        /// This means, a service remains consumable between
                        /// entities on a given system sweep
                        if (system.CanConsumeServices(_services)) {
                            for (auto& [entityID, entity] : _entities) {
                                if (system.CanConsumeComponents(entity)) {
                                    system.ConsumeEntity(entity, _services);
                                }
                            }
                        }
                    }

                    return;
                }

                /// @brief Start running the ECS main update loop in the
                /// current thread
                void Run() {
                    if (_mainLoop.joinable()) {
                        throw std::logic_error
                        ("ECS is already running in the dispatched thread");
                    }

                    _localContinue = true;
                    while (_localContinue) {
                        this->Sweep();
                    }
                }

                /// @brief Start running the ECS by dispatching a new thread
                /// running the main update loop
                void Dispatch() {
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

                /// @brief Wait until the ECS has stopped running on dispatched
                /// thread (via Dispatch)
                void AwaitStop() {
                    if (!_mainLoop.joinable()) {
                        throw std::logic_error
                        ("ECS is not running in the dispatched thread");
                    }

                    _mainLoop.join();
                }

                /// @brief Request that the ECS stop running (either on current
                /// thread or on dispatched thread via Dispatch)
                /// @return True if stop request was honored, 
                /// false otherwise
                bool RequestStop() {
                    if (_mainLoop.joinable()) {
                        return _mainLoop.request_stop();
                    }

                    _localContinue = false;
                    return true;
                }
        };
};
