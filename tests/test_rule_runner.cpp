#include "catch.hpp"

TEST_CASE("Single rule with compile a single, stale dependency", "[rule_runner]") {}

TEST_CASE("Test multirule with a stale dependencies", "[rule_runner]") {}

TEST_CASE("Test multirule with a non-stale dependencies", "[rule_runner]") {}

TEST_CASE("Single rule with compile a single, non-stale dependency", "[rule_runner]") {}

TEST_CASE("Test clean rule", "[rule_runner]") {}

TEST_CASE("Non-existant rule leads to error being thrown", "[rule_runner]") {}

TEST_CASE("Single rule chained command construction. No existing files", "[rule_runner]") {}

TEST_CASE("Single rule chained command construction. No recompile needed", "[rule_runner]") {}
