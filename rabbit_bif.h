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

#define BIF_NAME(id, name) #name,

const char *bif_names[] = {BIF_TABLE(BIF_NAME)};

#undef BIF_NAME

#endif
