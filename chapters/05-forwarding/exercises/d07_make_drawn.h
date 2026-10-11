#pragma once

#include <memory>

#include "shapes.h"
#include "tut/todo.h"

namespace ch05
{

/// Constructs a shape of type S from @p args, adds it to @p canvas and returns it:
///     auto circle = make_drawn<Circle>(canvas, 2.0);
template <typename S, typename... Args>
std::shared_ptr<S> make_drawn(Canvas& canvas, Args&&... args)
{
    TUT_TODO(canvas, args...);
}

}  // namespace ch05
