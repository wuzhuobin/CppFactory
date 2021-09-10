/**
 * @file factory_test.cpp
 * @author wuzhuobin (jiejin2022@163.com)
 * @brief
 * @version 0.1
 * @date 2021-04-09
 *
 * @copyright Copyright (c) 2021 WUZHUOBIN.
 *
 */

#include "util/factory.hpp"

// gtest
#include <gtest/gtest.h>

// std
#include <memory>
#include <type_traits>

using namespace ::testing;
using namespace chronos;
using namespace chronos::internal;

template <typename TFake>
class chronos_internal_FactoryTest0 : public Test
{
protected:
    using FactoryFake = typename TFake::FactoryFake;
    using FactoryFakeUnique = typename TFake::FactoryFakeUnique;

    void TearDown() override
    {
        FactoryFake::instance()->clear();
        FactoryFakeUnique::instance()->clear();
    }
};

class IFakeProduct
{
public:
    virtual ~IFakeProduct() = default;
};

template <typename TFake, typename... Args>
class IFakeProductT : public IFakeProduct
{
public:
    using Base = IFakeProduct;
    const static std::function<std::unique_ptr<Base>(Args&&...)> GMakeUnique;
    const static std::function<Base*(Args&&...)> GNew;
};

template <typename TFake, typename... Args>
const std::function<std::unique_ptr<typename IFakeProductT<TFake, Args...>::Base>(Args&&...)>
    IFakeProductT<TFake, Args...>::GMakeUnique = &std::make_unique<TFake, Args&&...>;

template <typename TFake, typename... Args>
const std::function<typename IFakeProductT<TFake, Args...>::Base*(Args&&...)> IFakeProductT<TFake, Args...>::GNew =
    [](Args&&... args) { return new TFake(std::forward<Args>(args)...); };

class FakeProductWithZeroArg : public IFakeProductT<FakeProductWithZeroArg>
{
public:
    using FactoryFake = Factory<Base*>;
    using FactoryFakeUnique = Factory<std::unique_ptr<Base>>;
    using RegistrarFake = Registrar<FakeProductWithZeroArg, std::unique_ptr<Base>>;

    // Same as the MACRO
    // static constexpr const char* classname() { return "FakeProductWithZeroArg"; }
    PRODUCT_CLASSNAME(FakeProductWithZeroArg);
    static std::unique_ptr<Base> create() { return std::make_unique<FakeProductWithZeroArg>(); }
};

class FakeProductWithTwoArgs : public IFakeProductT<FakeProductWithTwoArgs, char, int>
{
public:
    using FactoryFake = Factory<Base*, char, int>;
    using FactoryFakeUnique = Factory<std::unique_ptr<Base>, char, int>;
    using RegistrarFake = Registrar<FakeProductWithTwoArgs, std::unique_ptr<Base>, char, int>;
    char mArg0 = '\0';
    int mArg1 = 0;

    // Same as the MACRO
    // PRODUCT_CLASSNAME(FakeProductWithTwoArgs);
    static constexpr const char* classname() { return "FakeProductWithTwoArgs"; }
    static std::unique_ptr<Base> create(char arg0, int arg1)
    {
        return std::make_unique<FakeProductWithTwoArgs>(arg0, arg1);
    }

    FakeProductWithTwoArgs(char arg0, int arg1) : mArg0(arg0), mArg1(arg1) {}
};

using FakeProducts = Types<FakeProductWithZeroArg>;

TYPED_TEST_SUITE(chronos_internal_FactoryTest0, FakeProducts);

TYPED_TEST(chronos_internal_FactoryTest0, contains_ShouldReturnFalse_WhenNotRegistered)
{
    // Arrange
    std::string name("FakeProduct");
    auto* factory = TypeParam::FactoryFake::instance();
    auto* factoryUnique = TypeParam::FactoryFakeUnique::instance();

    // Act

    // Assert
    ASSERT_FALSE(factory->contains(name));
    ASSERT_FALSE(factoryUnique->contains(name));
}

TYPED_TEST(chronos_internal_FactoryTest0, contains_ShouldReturnTrue_WhenRegistered)
{
    // Arrange
    std::string name("FakeProduct");
    auto* factory = TypeParam::FactoryFake::instance();
    auto* factoryUnique = TypeParam::FactoryFakeUnique::instance();
    auto builder = TypeParam::GNew;
    auto builderUnique = TypeParam::GMakeUnique;

    // Act
    factory->registerObjectFactory(name, builder);
    factoryUnique->registerObjectFactory(name, builderUnique);

    // Assert
    ASSERT_TRUE(factory->contains(name));
    ASSERT_TRUE(factoryUnique->contains(name));
}

TYPED_TEST(chronos_internal_FactoryTest0, contains_ShouldReturnTrue_WhenRegistrarUsed)
{
    // Arrange
    auto* factoryUnique = TypeParam::FactoryFakeUnique::instance();

    // Act
    typename TypeParam::RegistrarFake registrar(TypeParam::create);

    // Assert
    ASSERT_TRUE(factoryUnique->contains(TypeParam::classname()));
}

TYPED_TEST(chronos_internal_FactoryTest0, contains_ShouldReturnFalse_WhenRegistrarNotUsed)
{
    // Arrange
    auto* factoryUnique = TypeParam::FactoryFakeUnique::instance();

    // Assert
    ASSERT_FALSE(factoryUnique->contains(TypeParam::classname()));
}

class chronos_internal_FactoryTest1 : public Test
{
protected:
    using FactoryFakeProductWithZeroArg = typename FakeProductWithZeroArg::FactoryFake;
    using FactoryFakeProductWithZeroArgUnique = typename FakeProductWithZeroArg::FactoryFakeUnique;
    using FactoryFakeProductWithTwoArgs = typename FakeProductWithTwoArgs::FactoryFake;
    using FactoryFakeProductWithTwoArgsUnique = typename FakeProductWithTwoArgs::FactoryFakeUnique;

    void TearDown() override
    {
        FactoryFakeProductWithZeroArg::instance()->clear();
        FactoryFakeProductWithZeroArgUnique::instance()->clear();
        FactoryFakeProductWithTwoArgs::instance()->clear();
        FactoryFakeProductWithTwoArgsUnique::instance()->clear();
    }
};

TEST_F(chronos_internal_FactoryTest1, create_ShouldReturnValidPointer_When_Registered)
{
    // Arrange
    std::string name("FakeProduct");

    auto* factoryZeroArg = FakeProductWithZeroArg::FactoryFake::instance();
    auto* factoryZeroArgUnique = FakeProductWithZeroArg::FactoryFakeUnique::instance();
    auto builderZeroArg = FakeProductWithZeroArg::GNew;
    auto builderZeroArgUnique = FakeProductWithZeroArg::GMakeUnique;

    factoryZeroArg->registerObjectFactory(name, builderZeroArg);
    factoryZeroArgUnique->registerObjectFactory(name, builderZeroArgUnique);

    auto* factoryTwoArgs = FakeProductWithTwoArgs::FactoryFake::instance();
    auto* factoryTwoArgsUnique = FakeProductWithTwoArgs::FactoryFakeUnique::instance();
    auto builderTwoArgs = FakeProductWithTwoArgs::GNew;
    auto builderTwoArgsUnique = FakeProductWithTwoArgs::GMakeUnique;

    factoryTwoArgs->registerObjectFactory(name, builderTwoArgs);
    factoryTwoArgsUnique->registerObjectFactory(name, builderTwoArgsUnique);

    // Act
    auto* productZeroArg = factoryZeroArg->create(name);
    auto productZeroArgUnique = factoryZeroArgUnique->create(name);
    auto isInterfaceZeroArg = std::is_same<IFakeProduct*, decltype(productZeroArg)>::value;
    auto isConcreteZeroArg = std::is_same<FakeProductWithZeroArg*, decltype(productZeroArg)>::value;
    auto isInterfaceZeroArgUnique = std::is_same<std::unique_ptr<IFakeProduct>, decltype(productZeroArgUnique)>::value;
    auto isConcreteZeroArgUnique =
        std::is_same<std::unique_ptr<FakeProductWithZeroArg>, decltype(productZeroArgUnique)>::value;

    auto* productTwoArgs = factoryTwoArgs->create(name, 'a', 1);
    auto productTwoArgsUnique = factoryTwoArgsUnique->create(name, 'a', 1);
    auto isInterfaceTwoArgs = std::is_same<IFakeProduct*, decltype(productTwoArgs)>::value;
    auto isConcreteTwoArgs = std::is_same<FakeProductWithTwoArgs*, decltype(productTwoArgs)>::value;
    auto isInterfaceTwoArgsUnique = std::is_same<std::unique_ptr<IFakeProduct>, decltype(productTwoArgsUnique)>::value;
    auto isConcreteTwoArgsUnique =
        std::is_same<std::unique_ptr<FakeProductWithTwoArgs>, decltype(productTwoArgsUnique)>::value;
    auto* productTwoArgsCasted = reinterpret_cast<FakeProductWithTwoArgs*>(productTwoArgs);
    auto* productTwoArgsUniqueCasted = reinterpret_cast<FakeProductWithTwoArgs*>(productTwoArgsUnique.get());

    // Assert
    ASSERT_TRUE(isInterfaceZeroArg);
    ASSERT_TRUE(isInterfaceZeroArgUnique);
    ASSERT_FALSE(isConcreteZeroArg);
    ASSERT_FALSE(isConcreteZeroArgUnique);
    ASSERT_TRUE(productZeroArg);
    ASSERT_TRUE(productZeroArgUnique);

    ASSERT_TRUE(isInterfaceTwoArgs);
    ASSERT_TRUE(isInterfaceTwoArgsUnique);
    ASSERT_FALSE(isConcreteTwoArgs);
    ASSERT_FALSE(isConcreteTwoArgsUnique);
    ASSERT_TRUE(productTwoArgs);
    ASSERT_TRUE(productTwoArgsUnique);
    ASSERT_EQ('a', productTwoArgsCasted->mArg0);
    ASSERT_EQ(1, productTwoArgsCasted->mArg1);
    ASSERT_EQ('a', productTwoArgsUniqueCasted->mArg0);
    ASSERT_EQ(1, productTwoArgsUniqueCasted->mArg1);

    delete productZeroArg;
    delete productTwoArgs;
}

TEST_F(chronos_internal_FactoryTest1, create_ShouldReturnNullptr_When_NotRegistered)
{
    // Arrange
    std::string name("FakeProduct");

    auto* factoryZeroArg = FakeProductWithZeroArg::FactoryFake::instance();
    auto* factoryZeroArgUnique = FakeProductWithZeroArg::FactoryFakeUnique::instance();
    auto builderZeroArg = FakeProductWithZeroArg::GNew;
    auto builderZeroArgUnique = FakeProductWithZeroArg::GMakeUnique;

    auto* factoryTwoArgs = FakeProductWithTwoArgs::FactoryFake::instance();
    auto* factoryTwoArgsUnique = FakeProductWithTwoArgs::FactoryFakeUnique::instance();
    auto builderTwoArgs = FakeProductWithTwoArgs::GNew;
    auto builderTwoArgsUnique = FakeProductWithTwoArgs::GMakeUnique;

    // Act
    auto* productZeroArg = factoryZeroArg->create(name);
    auto productZeroArgUnique = factoryZeroArgUnique->create(name);
    auto isInterfaceZeroArg = std::is_same<IFakeProduct*, decltype(productZeroArg)>::value;
    auto isConcreteZeroArg = std::is_same<FakeProductWithZeroArg*, decltype(productZeroArg)>::value;
    auto isInterfaceZeroArgUnique = std::is_same<std::unique_ptr<IFakeProduct>, decltype(productZeroArgUnique)>::value;
    auto isConcreteZeroArgUnique =
        std::is_same<std::unique_ptr<FakeProductWithZeroArg>, decltype(productZeroArgUnique)>::value;

    auto* productTwoArgs = factoryTwoArgs->create(name, 'a', 1);
    auto productTwoArgsUnique = factoryTwoArgsUnique->create(name, 'a', 1);
    auto isInterfaceTwoArgs = std::is_same<IFakeProduct*, decltype(productTwoArgs)>::value;
    auto isConcreteTwoArgs = std::is_same<FakeProductWithTwoArgs*, decltype(productTwoArgs)>::value;
    auto isInterfaceTwoArgsUnique = std::is_same<std::unique_ptr<IFakeProduct>, decltype(productTwoArgsUnique)>::value;
    auto isConcreteTwoArgsUnique =
        std::is_same<std::unique_ptr<FakeProductWithTwoArgs>, decltype(productTwoArgsUnique)>::value;

    // Assert
    ASSERT_TRUE(isInterfaceZeroArg);
    ASSERT_TRUE(isInterfaceZeroArgUnique);
    ASSERT_FALSE(isConcreteZeroArg);
    ASSERT_FALSE(isConcreteZeroArgUnique);
    ASSERT_FALSE(productZeroArg);
    ASSERT_FALSE(productZeroArgUnique);

    ASSERT_TRUE(isInterfaceTwoArgs);
    ASSERT_TRUE(isInterfaceTwoArgsUnique);
    ASSERT_FALSE(isConcreteTwoArgs);
    ASSERT_FALSE(isConcreteTwoArgsUnique);
    ASSERT_FALSE(productTwoArgs);
    ASSERT_FALSE(productTwoArgsUnique);
}

TEST_F(chronos_internal_FactoryTest1, create_ShouldReturnValidPointer_When_RegistrarUsed)
{
    // Arrange
    auto createWithZeroArg = &FakeProductWithZeroArg::create;
    auto createWithTwoArgs = &FakeProductWithTwoArgs::create;

    // If the variable is anonymous, below clang error will show up.
    // Parentheses were disambiguated as redundant parentheses around declaration of variable named 'createWithZeroArg'
    // (fixes available)clang(-Wvexing-parse)
    FakeProductWithZeroArg::RegistrarFake registrarZeroArg(createWithZeroArg);
    // Suppress the unused variable clang error.
    (void)registrarZeroArg;
    FakeProductWithTwoArgs::RegistrarFake registrarTwoArgs(createWithTwoArgs);
    (void)registrarTwoArgs;

    // Act
    auto productZeroArgUnique = createUnique<IFakeProduct>(FakeProductWithZeroArg::classname());
    auto isInterfaceZeroArgUnique = std::is_same<std::unique_ptr<IFakeProduct>, decltype(productZeroArgUnique)>::value;
    auto isConcreteZeroArgUnique =
        std::is_same<std::unique_ptr<FakeProductWithZeroArg>, decltype(productZeroArgUnique)>::value;

    auto productTwoArgsUnique = createUnique<IFakeProduct>(FakeProductWithTwoArgs::classname(), 'a', 1);
    auto isInterfaceTwoArgsUnique = std::is_same<std::unique_ptr<IFakeProduct>, decltype(productTwoArgsUnique)>::value;
    auto isConcreteTwoArgsUnique =
        std::is_same<std::unique_ptr<FakeProductWithZeroArg>, decltype(productTwoArgsUnique)>::value;
    auto* productTwoArgsUniqueCasted = reinterpret_cast<FakeProductWithTwoArgs*>(productTwoArgsUnique.get());

    // Assert
    ASSERT_TRUE(isInterfaceZeroArgUnique);
    ASSERT_FALSE(isConcreteZeroArgUnique);
    ASSERT_TRUE(productZeroArgUnique);

    ASSERT_TRUE(isInterfaceTwoArgsUnique);
    ASSERT_FALSE(isConcreteTwoArgsUnique);
    ASSERT_TRUE(productTwoArgsUnique);
    ASSERT_EQ('a', productTwoArgsUniqueCasted->mArg0);
    ASSERT_EQ(1, productTwoArgsUniqueCasted->mArg1);
}

TEST_F(chronos_internal_FactoryTest1, create_ShouldReturnNullptr_When_RegistrarNotUsed)
{
    // Arrange

    // Act
    auto productZeroArgUnique = createUnique<IFakeProduct>(FakeProductWithZeroArg::classname());
    auto isInterfaceZeroArgUnique = std::is_same<std::unique_ptr<IFakeProduct>, decltype(productZeroArgUnique)>::value;
    auto isConcreteZeroArgUnique =
        std::is_same<std::unique_ptr<FakeProductWithZeroArg>, decltype(productZeroArgUnique)>::value;

    auto productTwoArgsUnique = createUnique<IFakeProduct>(FakeProductWithTwoArgs::classname(), 'a', 1);
    auto isInterfaceTwoArgsUnique = std::is_same<std::unique_ptr<IFakeProduct>, decltype(productTwoArgsUnique)>::value;
    auto isConcreteTwoArgsUnique =
        std::is_same<std::unique_ptr<FakeProductWithZeroArg>, decltype(productTwoArgsUnique)>::value;

    // Assert
    ASSERT_TRUE(isInterfaceZeroArgUnique);
    ASSERT_FALSE(isConcreteZeroArgUnique);
    ASSERT_FALSE(productZeroArgUnique);

    ASSERT_TRUE(isInterfaceTwoArgsUnique);
    ASSERT_FALSE(isConcreteTwoArgsUnique);
    ASSERT_FALSE(productTwoArgsUnique);
}
