#ifndef B8AFA536_49F1_43EA_BB45_268A7263E9FC
#define B8AFA536_49F1_43EA_BB45_268A7263E9FC

#include <stdint.h>

// A common file to define workloads.

uint64_t compute_fibonacci(uint64_t value) {
  if (value <= 2) {
    return 1;
  }
  return compute_fibonacci(value - 1) + compute_fibonacci(value - 2);
}

void compiler_must_not_elide(const uint64_t t) {
  asm volatile("" : : "r"(t));
}


#endif /* B8AFA536_49F1_43EA_BB45_268A7263E9FC */
