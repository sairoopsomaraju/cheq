#ifndef STACK_USAGE_H
#define STACK_USAGE_H

#include <array>
#include <cstdint>
#include <utility>

#define STACK_USAGE(FUNC, USAGE)	std::make_pair<const char *, uint64_t>(FUNC, USAGE)

constexpr const std::array StackUsageArr{
#include "stack_usage.inc"
};

#undef STACK_USAGE

#endif /* STACK_USAGE_H */
