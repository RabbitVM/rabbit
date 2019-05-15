#ifndef RABBIT_BIF_H
#define RABBIT_BIF_H

#define BIF_TABLE(V)                                                           \
  V(Hello, hello)

#define BIF_ID(id, name) id,

// clang-format off
enum BifId {
  BIF_TABLE(BIF_ID)
  kNumBifs,
  kInvalidBif,
};
// clang-format on

#undef BIF_ID

#endif
