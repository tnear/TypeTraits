// Type trait experimentation
// Examples include template specializations for numeric types, accumulators, monoids,
// template policies, Const/Volatile (CV)-qualifiers, SFINAE, and C++17 void-type
// Ideas from C++ Templates, D. Vandevoorde

#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <utility>
#include <functional>

template <typename T>
struct AccumulationTraits;

template <>
struct AccumulationTraits<char>
{
    using AccT = int;
    static const AccT zero = 0;
};

template <>
struct AccumulationTraits<short>
{
    using AccT = int;
    static const AccT zero = 0;
};

template <>
struct AccumulationTraits<int>
{
    using AccT = long;
    static const AccT zero = 0;
};

template <>
struct AccumulationTraits<unsigned>
{
    using AccT = unsigned long;
    static const AccT zero = 0;
};

template <>
struct AccumulationTraits<float>
{
    using AccT = double;
    static constexpr AccT zero = 0;
};

template <typename T>
auto accum(const T* beg, const T* end)
{
    using AccT = typename AccumulationTraits<T>::AccT;
    AccT total = AccumulationTraits<T>::zero;

    while (beg != end)
    {
        total += *beg;
        ++beg;
    }

    return total;
}

template <typename T>
T accumBasic(const T* beg, const T* end)
{
    T total{};
    while (beg != end)
    {
        total += *beg;
        ++beg;
    }

    return total;
}

void testAccum()
{
    int v[] = { 1, 3, 5, 7 };
    auto total = accum(std::begin(v), std::end(v));
    std::cout << total << "\n";

    char name[] = "templates";
    total = accum(std::begin(name), std::end(name));
    std::cout << total << "\n";
}

struct SumPolicy
{
    template <typename T1, typename T2>
    static void accumulate(T1 &total, const T2 &value)
    {
        total += value;
    }
};

struct MultPolicy
{
    template <typename T1, typename T2>
    static void accumulate(T1 &total, const T2 &value)
    {
        total *= value;
    }
};

template <typename T, typename Policy = SumPolicy, typename Traits = AccumulationTraits<T>>
auto accumPolicy(const T* beg, const T* end)
{
    using AccT = typename Traits::AccT;
    AccT total = Traits::zero;
    while (beg != end)
    {
        Policy::accumulate(total, *beg);
        ++beg;
    }

    return total;
}

void testAccumPolicy()
{
    int v[] = { 1, 3, 5, 7 };
    auto result = accumPolicy(std::begin(v), std::end(v));
    std::cout << result << "\n";
    result = accumPolicy<int, MultPolicy>(std::begin(v), std::end(v));
    std::cout << result << "\n";
}

template <typename T1, typename T2>
class SumPolicyTemplate
{
public:
    static void accumulate(T1& total, const T2& value)
    {
        total += value;
    }
};

template <typename T,
    template <typename, typename> class Policy = SumPolicyTemplate,
    typename Traits = AccumulationTraits<T>>
    auto accumTemplateTemplate(const T * beg, const T * end)
{
    using AccT = typename Traits::AccT;
    AccT total = Traits::zero;
    while (beg != end)
    {
        Policy<AccT, T>::accumulate(total, *beg);
        ++beg;
    }
    return total;
};

void testAccumTemplate()
{
    int v[] = { 1, 3, 5, 7 };
    auto result = accumTemplateTemplate<int, SumPolicyTemplate>(std::begin(v), std::end(v));

    std::cout << result << " ";
}

template <typename T>
struct RemoveConstT
{
    using Type = T;
};

template <typename T>
struct RemoveConstT<const T>
{
    using Type = T;
};

template <typename T>
using RemoveConst = typename RemoveConstT<T>::Type;

template <typename T>
struct RemoveVolatileT
{
    using Type = T;
};

template <typename T>
struct RemoveCVT : RemoveConstT<typename RemoveVolatileT<T>::Type>
{
};

template <typename T>
using RemoveCV = typename RemoveCVT<T>::Type;

/*
template <typename T1, typename T2>
struct IsSameT
{
    static constexpr bool value = false;
};

template <typename T>
struct IsSameT<T, T> // partial specialization
{
    static constexpr bool value = true;
};
*/

template <bool val>
struct BoolConstant
{
    using Type = BoolConstant<val>;
    static constexpr bool value = val;
};

using TrueType = BoolConstant<true>;
using FalseType = BoolConstant<false>;

template <typename T1, typename T2>
struct IsSameT : FalseType
{
};

template <typename T>
struct IsSameT<T, T> : TrueType
{
};

template <typename T>
void foolImpl(T, TrueType)
{
    std::cout << "fooImpl(T, true)\n";
}

template <typename T>
void fooImpl(T, FalseType)
{
    std::cout << "fooImpl(T, false)\n";
}

template <typename T>
void foo(T t)
{
    //fooImpl(t, IsSameT<T, int>{});
}

void testFoo()
{
    foo(10);
    //foo(10.1);
}

// SFINAE
template <typename T>
struct IsDefaultConstructibleHelper
{
private:
    template <typename U, typename = decltype(U())>
    static std::true_type test(void*);

    template <typename>
    static std::false_type test(...);

public:
    using Type = decltype(test<T>(nullptr));
};

template <typename T>
struct IsDefaultConstructibleT : IsDefaultConstructibleHelper<T>::Type
{
};

struct S
{
    S() = delete;
};

void testIsDefaultConstructible()
{
    std::cout << IsDefaultConstructibleT<int>::value << "\n";
    std::cout << IsDefaultConstructibleT<S>::value << "\n";
}

template <typename, typename, typename = std::void_t<>>
struct HasPlusT : std::false_type
{
};

template <typename T1, typename T2>
struct HasPlusT<T1, T2, std::void_t<decltype(std::declval<T1>() + std::declval<T2>())>>
    : std::true_type
{
};

template <typename ...>
using VoidT = void;

template <typename, typename = VoidT<>>
struct HasSizeTypeT : std::false_type
{
};

template <typename T>
struct HasSizeTypeT<T, VoidT<typename T::size_type>> : std::true_type
{
};

struct CX
{
    using size_type = std::size_t;
};

void testHasSizeTypeT()
{
    std::cout << HasSizeTypeT<int>::value << "\n";
    std::cout << HasSizeTypeT<CX>::value << "\n";
}
