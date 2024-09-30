#ifndef DEBUG_H
#define DEBUG_H
#include <iostream>

#ifdef DEBUG
#define CERR(x) std::cerr << x
#define CERR_ENDL(x) std::cerr << x << std::endl
#define COUT(x) std::cout << x
#define COUT_ENDL(x) std::cout << x << std::endl
#else
#define CERR(x)                                                                                                        \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
#define CERR_ENDL(x)                                                                                                   \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
#define COUT(x)                                                                                                        \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
#define COUT_ENDL(x)                                                                                                   \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
#endif

#endif