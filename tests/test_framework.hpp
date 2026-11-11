/**
 * @file test_framework.hpp
 * @brief Minimal, dependency-free C++23 unit test helpers.
 */
#pragma once

#include <format>
#include <iostream>
#include <string_view>

inline int tf_checks_run = 0;
inline int tf_checks_failed = 0;
inline int tf_tests_run = 0;
inline int tf_tests_failed = 0;

#define TF_FAIL_(...)                                             \
	do {                                                          \
		++tf_checks_failed;                                       \
		std::cout << std::format("  {}:{}: ", __FILE__, __LINE__) \
				  << std::format(__VA_ARGS__) << '\n';            \
	} while(0)

/** Fails the current test if cond is false (the test keeps running). */
#define CHECK(cond)                              \
	do {                                         \
		++tf_checks_run;                         \
		if(!(cond)) {                            \
			TF_FAIL_("CHECK({}) failed", #cond); \
		}                                        \
	} while(0)

/** Compares two values for equality. */
#define CHECK_EQ(expected, actual)                           \
	do {                                                     \
		++tf_checks_run;                                     \
		if(!((expected) == (actual))) {                      \
			TF_FAIL_("{} == {} failed", #expected, #actual); \
		}                                                    \
	} while(0)

/** Runs a void fn() test function and reports PASS/FAIL. */
#define RUN_TEST(fn)                          \
	do {                                      \
		int tf_before_ = tf_checks_failed;    \
		++tf_tests_run;                       \
		fn();                                 \
		if(tf_checks_failed == tf_before_) {  \
			std::cout << "  PASS  " #fn "\n"; \
		} else {                              \
			std::cout << "  FAIL  " #fn "\n"; \
			++tf_tests_failed;                \
		}                                     \
	} while(0)

/** Prints a summary; use as return TEST_REPORT(); at the end of main(). */
inline int TEST_REPORT() {
	std::cout << std::format("  {}/{} tests passed ({} checks, {} failed)\n",
							 tf_tests_run - tf_tests_failed, tf_tests_run,
							 tf_checks_run, tf_checks_failed);
	std::cout.flush();
	return tf_tests_failed == 0 ? 0 : 1;
}
