// Copyright (c) 2014, Tamas Csala

#ifndef OGLWRAP_ENUMS_PIXEL_DATA_TYPE_H_
#define OGLWRAP_ENUMS_PIXEL_DATA_TYPE_H_

#include "../config.h"

namespace OGLWRAP_NAMESPACE_NAME {
inline namespace enums {

enum class PixelDataType : GLenum {
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_BYTE)
  kUnsignedByte = GL_UNSIGNED_BYTE,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_BYTE)
  kByte = GL_BYTE,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_SHORT)
  kUnsignedShort = GL_UNSIGNED_SHORT,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_SHORT)
  kShort = GL_SHORT,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_INT)
  kUnsignedInt = GL_UNSIGNED_INT,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_INT)
  kInt = GL_INT,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_HALF_FLOAT)
  kHalfFloat = GL_HALF_FLOAT,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_FLOAT)
  kFloat = GL_FLOAT,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_BYTE_3_3_2)
  kUnsignedByte332 = GL_UNSIGNED_BYTE_3_3_2,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_BYTE_2_3_3_REV)
  kUnsignedByte233Rev = GL_UNSIGNED_BYTE_2_3_3_REV,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_SHORT_5_6_5)
  kUnsignedShort565 = GL_UNSIGNED_SHORT_5_6_5,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_SHORT_5_6_5_REV)
  kUnsignedShort565Rev = GL_UNSIGNED_SHORT_5_6_5_REV,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_SHORT_4_4_4_4)
  kUnsignedShort4444 = GL_UNSIGNED_SHORT_4_4_4_4,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_SHORT_4_4_4_4_REV)
  kUnsignedShort4444Rev = GL_UNSIGNED_SHORT_4_4_4_4_REV,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_SHORT_5_5_5_1)
  kUnsignedShort5551 = GL_UNSIGNED_SHORT_5_5_5_1,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_SHORT_1_5_5_5_REV)
  kUnsignedShort1555Rev = GL_UNSIGNED_SHORT_1_5_5_5_REV,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_INT_8_8_8_8)
  kUnsignedInt8888 = GL_UNSIGNED_INT_8_8_8_8,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_INT_8_8_8_8_REV)
  kUnsignedInt8888Rev = GL_UNSIGNED_INT_8_8_8_8_REV,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_INT_10_10_10_2)
  kUnsignedInt1010102 = GL_UNSIGNED_INT_10_10_10_2,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_INT_2_10_10_10_REV)
  kUnsignedInt2101010Rev = GL_UNSIGNED_INT_2_10_10_10_REV,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_INT_24_8)
  kUnsignedInt248 = GL_UNSIGNED_INT_24_8,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_INT_10F_11F_11F_REV)
  kUnsignedInt10F11F11FRev = GL_UNSIGNED_INT_10F_11F_11F_REV,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_UNSIGNED_INT_5_9_9_9_REV)
  kUnsignedInt5999Rev = GL_UNSIGNED_INT_5_9_9_9_REV,
#endif
#if OGLWRAP_DEFINE_EVERYTHING || defined(GL_FLOAT_32_UNSIGNED_INT_24_8_REV)
  kFloat32UnsignedInt248Rev = GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
#endif
};

} // namespace enums

} // namespace oglwrap

#endif
