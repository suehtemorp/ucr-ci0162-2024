#ifndef TEMPLATES_HPP
#define TEMPLATES_HPP

#include <concepts>
#include <type_traits>

/// @brief Whether a type is present on a given pack
/// @tparam MainType Type to check
/// @tparam ...OtherTypes Type pack to check 
template<typename MainType, typename... OtherTypes>
struct is_any_from : std::disjunction<std::is_same<MainType, OtherTypes>...> {};

/// @brief Whether a type is present on a given pack
/// @tparam MainType Type to check
/// @tparam ...OtherTypes Type pack to check
template<typename MainType, typename... OtherTypes>
concept AnyFrom = is_any_from<MainType, OtherTypes...>::value;

/// @brief Whether all types within a pack are distinct from each other
/// @tparam Types Parameter pack to check
template<typename... Types>
struct are_distinct : std::true_type {};

template <typename Type>
struct are_distinct<Type> : std::true_type {};

template <typename FirstType, typename... OtherTypes>
struct are_distinct<FirstType, OtherTypes...> : std::conjunction<
    std::negation<
        is_any_from<FirstType, OtherTypes...>
    >,
    are_distinct<OtherTypes...>
> {};

/// @brief Whether all types within a pack are distinct from each other
/// @tparam Types Parameter pack to check
template<typename... Types>
concept Distinct = are_distinct<Types...>::value;

#endif
