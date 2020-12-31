#pragma once

#ifdef CORE_LIB
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif