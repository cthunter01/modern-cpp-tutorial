#pragma once

#include <memory>
#include <utility>

#include "shapes.h"

namespace ch05
{

//! [make-drawn]
/// Constructs a shape of type S from @p args, adds it to @p canvas and returns it:
///     auto circle = make_drawn<Circle>(canvas, 2.0);
template <typename S, typename... Args>
std::shared_ptr<S> make_drawn(Canvas& canvas, Args&&... args)
{
    std::shared_ptr<S> shape = std::make_shared<S>(std::forward<Args>(args)...);
    canvas.shapes.push_back(shape);  // a copy of the pointer, converted to shared_ptr<Shape>
    return shape;                    // the caller gets the exact type back
}
//! [make-drawn]

}  // namespace ch05
