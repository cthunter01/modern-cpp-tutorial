#pragma once

#include <string>
#include <utility>

// Given, for problem 3. Nothing to change here.

namespace review_a
{

class Widget
{
public:
    Widget()                         = default;
    Widget(const Widget&)            = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&)                 = delete;
    Widget& operator=(Widget&&)      = delete;
    virtual ~Widget()                = default;

    [[nodiscard]] virtual std::string Describe() const = 0;
};

class Button final : public Widget
{
public:
    explicit Button(std::string caption) : label(std::move(caption)) { }

    [[nodiscard]] std::string Describe() const override { return "button '" + label + "'"; }

private:
    std::string label;
};

class Slider final : public Widget
{
public:
    Slider(int lowest, int highest) : low(lowest), high(highest) { }

    [[nodiscard]] std::string Describe() const override
    {
        return "slider " + std::to_string(low) + ".." + std::to_string(high);
    }

private:
    int low;
    int high;
};

}  // namespace review_a
