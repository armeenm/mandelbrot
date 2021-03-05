#pragma once

auto constexpr inline profiling = false;

#ifdef N_DEBUG
auto constexpr inline is_debug = false;
#else
auto constexpr inline is_debug = true;
#endif
