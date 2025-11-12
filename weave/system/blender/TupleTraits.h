#pragma once

#include <tuple>
#include <typeinfo>
#include <typeindex>
#include <type_traits>

namespace weave {

template<typename T>
struct TupleTraits {
    static constexpr bool isTuple = false;
    static constexpr size_t size = 0;

    template <size_t N>
    using tuple_element_t = T;
};

template<typename ...Types>
struct TupleTraits<std::tuple<Types...>> {
    static constexpr bool isTuple = true;
    static constexpr size_t size = std::tuple_size_v<std::tuple<Types...>>;

    template <size_t N>
    using tuple_element_t = std::tuple_element_t<N, std::tuple<Types...>>;
};

template <typename T, size_t N = 0>
std::type_index getTypeByIndex(size_t n) {
    using TupleTrait = TupleTraits<std::decay_t<T>>;
    if constexpr (TupleTrait::isTuple && TupleTrait::size > 0) {
        if (n < TupleTrait::size) {
            if (n == 0) {
                using ElementType = TupleTrait::template tuple_element_t<N>;
                return typeid(ElementType);
            }
            else {
                if constexpr (N + 1 < TupleTrait::size) {
                    return getTypeByIndex<T, N + 1>(n - 1);
                }
            }
        }
    }
    else {
        if (n == 0) {
            return typeid(T);
        }
    }

    return typeid(void);
}

template <size_t N = 0, typename T>
std::pair<std::type_index, void*> getTypeAndPtrByIndex(T&& valueOrTuple, size_t n) {
    using TupleTrait = TupleTraits<std::decay_t<T>>;
    if constexpr (TupleTrait::isTuple && TupleTrait::size > 0) {
        if (n < TupleTrait::size) {
            if (n == 0) {
                using ElementType = TupleTrait::template tuple_element_t<N>;
                return { typeid(ElementType), (void*)&std::get<N>(valueOrTuple)};
            }
            else {
                if constexpr (N + 1 < TupleTrait::size) {
                    return getTypeAndPtrByIndex<N + 1>(std::forward<T>(valueOrTuple), n - 1);
                }
            }
        }
    }
    else {
        if (n == 0) {
            return { typeid(T), (void*)&valueOrTuple };
        }   
    }

    return { typeid(void), nullptr };
}

template <size_t N = 0, typename T, 
    std::enable_if_t<(TupleTraits<std::decay_t<T>>::isTuple), int> = 0>
std::pair<std::type_index, std::shared_ptr<void>> getTypeAndSharedPtrByIndex(T&& tuple, size_t n) {
    using TupleTrait = TupleTraits<std::decay_t<T>>;

    if constexpr (TupleTrait::isTuple && TupleTrait::size > 0) {
        if (n < TupleTrait::size) {
            if (n == 0) {
                using ElementType = TupleTrait::template tuple_element_t<N>;
                std::shared_ptr<void> ptr = std::get<N>(tuple);
                return { typeid(typename ElementType::element_type), ptr };
            }
            else {
                if constexpr (N + 1 < TupleTrait::size) {
                    return getTypeAndSharedPtrByIndex<N + 1>(std::forward<T>(tuple), n - 1);
                }
            }
        }
    }

    return { typeid(void), std::shared_ptr<void>{} };
}



template<typename TupleIn, template<typename...> class TupleOut>
struct use_variadic_types;

template<template<typename...> class TupleIn, template<typename...> class TupleOut, typename ...Types>
struct use_variadic_types<TupleIn<Types...>, TupleOut> {
    using type = TupleOut<Types...>;
};

template <typename TupleIn, template<typename...> class TupleOut>
using use_variadic_types_t = typename use_variadic_types<TupleIn, TupleOut>::type;



template<template<typename...> class Template, typename T>
struct is_specialization_of : std::false_type {};

template<template<typename...> class Template, typename ...Types>
struct is_specialization_of<Template, Template<Types...>> : std::true_type {};

template<template<typename...> class Template, typename ...Types>
constexpr bool is_specialization_of_v = is_specialization_of<Template, Types...>::value;


template <template <typename...> class Base, typename... Ts>
struct first_specialization_of;

template <template <typename...> class Base, typename T, typename... Ts>
struct first_specialization_of<Base, T, Ts...> {
    using type = std::conditional_t<
        is_specialization_of_v<Base, T>,
        T,
        typename first_specialization_of<Base, Ts...>::type
    >;
};

template <template <typename...> class Base>
struct first_specialization_of<Base> {
    using type = Base<>;
};

template <template <typename...> class Base, typename... Ts>
using first_specialization_of_t = typename first_specialization_of<Base, Ts...>::type;



template <template <typename...> class Base, typename... Ts>
struct count_specializations_of {
    static constexpr size_t value = ((is_specialization_of_v<Base, Ts> ? 1 : 0) + ...);
};

template <template <typename...> class Base>
struct count_specializations_of<Base> {
    static constexpr size_t value = 0;
};

template <template <typename...> class Base, typename... Ts>
constexpr size_t count_specializations_of_v = count_specializations_of<Base, Ts...>::value;



template <typename Base, typename... Ts>
struct first_base_of;

template <typename Base, typename T, typename... Ts>
struct first_base_of<Base, T, Ts...> {
    using type = std::conditional_t<
        std::is_base_of_v<Base, T>,
        T,
        typename first_base_of<Base, Ts...>::type
    >;
};

template <typename Base>
struct first_base_of<Base> {
    using type = Base;
};

template <typename Base, typename... Ts>
using first_base_of_t = typename first_base_of<Base, Ts...>::type;



template<typename T, typename... Ts>
struct contains_type {
    static constexpr bool value = (std::is_same_v<T, Ts> || ...);
};

template<typename T, typename... Ts>
constexpr bool contains_type_v = contains_type<T, Ts...>::value;

}
