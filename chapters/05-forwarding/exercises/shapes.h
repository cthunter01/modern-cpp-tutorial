#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "tut/tracked.h"

// Given, for drill 7. Nothing to change here.

namespace ch05
{

class Shape
{
public:
    Shape()                        = default;
    Shape(const Shape&)            = delete;
    Shape& operator=(const Shape&) = delete;
    Shape(Shape&&)                 = delete;
    Shape& operator=(Shape&&)      = delete;
    virtual ~Shape()               = default;

    [[nodiscard]] virtual double Area() const = 0;
};

class Circle final : public Shape
{
public:
    explicit Circle(double circleRadius) : radius(circleRadius) { }

    [[nodiscard]] double Area() const override { return 3.0 * radius * radius; }  // near enough
    [[nodiscard]] double Radius() const noexcept { return radius; }

private:
    double radius;
};

class Rectangle final : public Shape
{
public:
    Rectangle(double rectangleWidth, double rectangleHeight)
      : width(rectangleWidth), height(rectangleHeight)
    {
    }

    [[nodiscard]] double Area() const override { return width * height; }

private:
    double width;
    double height;
};

/// A shape with a caption and a payload, to see what happens to constructor arguments on their way
/// in.
class Labelled final : public Shape
{
public:
    Labelled(std::string text, tut::Tracked content)
      : label(std::move(text)), payload(std::move(content))
    {
    }

    [[nodiscard]] double              Area() const override { return 0.0; }
    [[nodiscard]] const std::string&  Label() const noexcept { return label; }
    [[nodiscard]] const tut::Tracked& Payload() const noexcept { return payload; }

private:
    std::string  label;
    tut::Tracked payload;
};

/// Everything that has been drawn, in order.
struct Canvas
{
    std::vector<std::shared_ptr<Shape>> shapes;
};

}  // namespace ch05
