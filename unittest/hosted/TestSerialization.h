#pragma once
#include <cstdint>
#include <array>

static bool test_value_bool = true;
static uint8_t tv_uint8   {5};
static uint16_t tv_uint16 {283};
static uint32_t tv_uint32 {929221};
static uint64_t tv_uint64 {2929329429};

static int8_t tv_int8 {-16};
static int16_t tv_int16 {-829};
static int32_t tv_int32 {-2312};

static float tv_float {8.2149214};
static float tv_sfloat = {-922.2321321};
static double tv_double {9.2132142141e8};
static double tv_sdouble {-2.2421e19};

static std::array<uint8_t, 512> test_array;
