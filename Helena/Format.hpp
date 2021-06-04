#ifndef HELENA_FORMAT_HPP
#define HELENA_FORMAT_HPP

#define FMT_HEADER_ONLY

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/color.h>

#define HF_FORMAT(...)      fmt::format(__VA_ARGS__)

#endif // HELENA_FORMAT_HPP
