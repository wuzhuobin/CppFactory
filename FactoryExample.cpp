/**
 * @file FactoryExample.cpp
 * @example Factory
 * @author wuzhuobin (jiejin2022@163.com)
 * @brief
 * @version 0.1
 * @date 2021-04-11
 *
 * @copyright Copyright (c) 2021 WUZHUOBIN
 *
 * An example to demostrate how to use the factory.
 */

#include "util/factory.hpp"

// std
#include <iostream>
#include <memory>

/**
 * @brief A Product interface.
 *
 */
class IProduct
{
public:
    virtual ~IProduct() = default;
    virtual void echo() = 0;
};

class ProductWithZeroParam : public IProduct
{
public:
    PRODUCT_CLASSNAME(ProductWithZeroParam);
    void echo() override { std::cout << "I am ProductWithZeroParam.\n"; }
};

PRODUCT_UNIQUE_REGISTRAR([]() { return std::make_unique<ProductWithZeroParam>(); }, ProductWithZeroParam, IProduct);
PRODUCT_REGISTRAR([]() { return new ProductWithZeroParam(); }, ProductWithZeroParam, IProduct);

class ProductWithTwoParams : public IProduct
{
public:
    PRODUCT_CLASSNAME(ProductWithTwoParams);
    static std::unique_ptr<IProduct> create(int arg0, char arg1)
    {
        return std::make_unique<ProductWithTwoParams>(arg0, arg1);
    }
    ProductWithTwoParams(int /*unused*/, char /*unused*/) {}
    void echo() override { std::cout << "I am ProductWithTwoParams.\n"; }
};

PRODUCT_UNIQUE_REGISTRAR([](int arg0, char arg1) { return std::make_unique<ProductWithTwoParams>(arg0, arg1); },
                         ProductWithTwoParams,
                         IProduct,
                         int,
                         char);

PRODUCT_REGISTRAR([](int arg0, char arg1) { return new ProductWithTwoParams(arg0, arg1); },
                  ProductWithTwoParams,
                  IProduct,
                  int,
                  char);

int main(int /*argc*/, char* /*argv*/[])
{
    auto p1 = createUnique<IProduct>("ProductWithZeroParam");
    std::cout << "p1 echo\n";
    p1->echo();

    auto* p2 = create<IProduct*>("ProductWithZeroParam");
    std::cout << "p2 echo\n";
    p2->echo();
    delete p2;

    auto p3 = createUnique<IProduct>("ProductWithTwoParams", 0, '\0');
    std::cout << "p3 echo\n";
    p3->echo();

    auto* p4 = create<IProduct*>("ProductWithTwoParams", 0, '\0');
    std::cout << "p4 echo\n";
    p4->echo();
    delete p4;

    auto p5 = createUnique<IProduct>("ProductWithTwoParams");
    std::cout << "p5 echo\n";
    std::cout << (p5 ? "p5 is not a nullptr.\n" : "p5 is a nullptr.\n");
    return 0;
}
