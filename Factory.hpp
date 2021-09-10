/**
 * @file factory.hpp
 * @author wuzhuobin (jiejin2022@163.com)
 * @brief
 * @version 0.1
 * @date 2021-04-09
 *
 * @copyright Copyright (c) 2021 WUZHUOBIN.
 *
 */

#ifndef FACTORY_HPP
#define FACTORY_HPP

// std
#include <functional>
#include <map>
#include <memory>

template <typename TBasePtr, typename... Args>
class Factory final
{
public:
    using TBuilder = std::function<TBasePtr(Args...)>;

    Factory(const Factory&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&&) = delete;
    void* operator new(::size_t) = delete;
    void operator delete(void*) = delete;

    static Factory* instance()
    {
        static Factory gFactory;
        return &gFactory;
    }

    void registerObjectFactory(const std::string& name, const TBuilder& builder) { mBuilderRegistry[name] = builder; }

    [[nodiscard]] bool contains(const std::string& name) const
    {
        return (mBuilderRegistry.find(name) != mBuilderRegistry.end());
    }

    template <typename... SameArgs>
    [[nodiscard]] TBasePtr create(const std::string& name, SameArgs&&... args) const
    {
        // In class template argument deduction, template parameter of a class template is never a forwarding reference.
        // So it has to be used as function template argument.
        static_assert(std::is_same<TBuilder, std::function<TBasePtr(SameArgs...)>>::value,
                      "Input arguments types should matched.");
        TBasePtr instance = nullptr;
        auto it = mBuilderRegistry.find(name);
        if (it != mBuilderRegistry.end())
        {
            instance = it->second(std::forward<SameArgs>(args)...);
        }
        return instance;
    }

    void clear() { mBuilderRegistry.clear(); }

private:
    Factory() = default;
    ~Factory() = default;

    std::map<std::string, TBuilder> mBuilderRegistry;
};

template <typename TBasePtr, typename... Args>
[[nodiscard]] static TBasePtr create(const std::string& name, Args&&... args)
{
    return Factory<TBasePtr, Args...>::instance()->create(name, std::forward<Args>(args)...);
}

template <typename TBase, typename... Args>
[[nodiscard]] static std::unique_ptr<TBase> createUnique(const std::string& name, Args&&... args)
{
    return create<std::unique_ptr<TBase>, Args...>(name, std::forward<Args>(args)...);
}

template <typename TProduct, typename TBasePtr, typename... Args>
class Registrar final
{
public:
    using TFactory = Factory<TBasePtr, Args...>;
    using TBuilder = std::function<TBasePtr(Args...)>;
    Registrar(const Registrar&) = delete;
    Registrar& operator=(const Registrar&) = delete;
    Registrar(Registrar&&) = delete;
    Registrar& operator=(const Registrar&&) = delete;
    void* operator new(::size_t) = delete;
    void operator delete(void*) = delete;

    Registrar(const TBuilder& create, const std::string& name = TProduct::classname())
    {
        TFactory::instance()->registerObjectFactory(name, create);
    }
};


// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PRODUCT_CLASSNAME(product) \
    [[nodiscard]] static constexpr const char* classname() { return #product; }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PRODUCT_UNIQUE_REGISTRAR(create, product, base, ...)                                 \
    static const chronos::internal::Registrar<product, std::unique_ptr<base>, ##__VA_ARGS__> \
        G##product##UniqueRegistrar(create);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PRODUCT_REGISTRAR(create, product, base, ...) \
    static const chronos::internal::Registrar<product, base*, ##__VA_ARGS__> G##product##Registrar(create);

#endif  //! FACTORY_HPP
