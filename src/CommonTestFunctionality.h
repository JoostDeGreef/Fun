#pragma once

#include "gtest/gtest.h"
using namespace testing;


//inline std::ostream& operator<<(std::ostream& stream, Vector2d const& v)
//{
//    return stream << "Vector2d(" << v[0] << "," << v[1] << ")";
//}
//
//inline std::ostream& operator<<(std::ostream& stream, Vector3d const& v)
//{
//    return stream << "Vector3d(" << v[0] << "," << v[1] << "," << v[2] << ")";
//}
//
//inline std::ostream& operator<<(std::ostream& stream, BoundingShape3d const& bs)
//{
//    switch (bs.GetType())
//    {
//    default:
//    case BoundingShape3d::Type::Unknown: return stream << "BoundingShape3d(Unknown)";
//    case BoundingShape3d::Type::Box: return stream << "BoundingShape3d(Box," << bs.GetMin() << "," << bs.GetMax() << ")";
//    case BoundingShape3d::Type::Ball: return stream << "BoundingShape3d(Ball," << bs.GetCenter() << "," << bs.GetRadius() << ")";
//    }
//}
