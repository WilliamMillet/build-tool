#include "catch.hpp"

// Action plan
// - Split command building stuff into a separate command building factory for SRP and testing ease
//     - There should be different building strategies which can be instantiated and held in the
//     rules as private variables
// - Write tests for command building
// - Edit lexer to allow for hyphens in identifier names

// class FileSystemGateway {
//      virtual exists(string);
//      virtual last_write_time();
//      virtual touch();
// }
//
// class RealFileSystem..
//
//
// struct MockFileEntry {
//  String name
//  Time write_time
//  size_t modification_count;
// }
//
//  /** Store a copy of this during tests to interact with it */
//  class MockFSGateway : public FileSystemGateway {
//    public
//      MockFSGateway();
//      override methods using mockfs
//    private:
//      Map<String, MockFileEntry> name to file entry
// }
//
// class ProcessRunner {
//  virtual int run(const std::vector<std::string>& cmd);
// }
// class PosixRunner : public ProcessRunner {
//     // Override run by actually running the process
// }
// class MockProcRunner : public ProcessRunner {
//     // MockProcRunner(std::string )
//     // Override run by doing the following
//     //  - Edit the last runt ime of the filesystem gateway
//     // Increment a modification counter in the mock fs gateway
// }

TEST_CASE("Single rule command construction", "[rule_runner]") {}

TEST_CASE("Multi rule command construction", "[rule_runner]") {}

TEST_CASE("Clean rule command construction", "[rule_runner]") {}

TEST_CASE("Single rule with compile a single, stale dependency", "[rule_runner]") {
    // TempDir tdir
    // MockProcRunner mock_pr(tdir)
    // Initialise graph {a: [b, c], b: [d, e]} where IDs are "{tdir}/{ltr}"
    // Config cfg
    // Create RuleRunner with mock_pr, graph and cfg
    //
    // Call run_rule
    // Assert all expected files exist in rule directory
    // Cleanup tdir
}

TEST_CASE("Test multirule with a stale dependencies", "[rule_runner]") {}

TEST_CASE("Test multirule with a non-stale dependencies", "[rule_runner]") {}

TEST_CASE("Single rule with compile a single, non-stale dependency", "[rule_runner]") {}

TEST_CASE("Test clean rule", "[rule_runner]") {}

TEST_CASE("Non-existant rule leads to error being thrown", "[rule_runner]") {}

TEST_CASE("Single rule chained command construction. No existing files", "[rule_runner]") {}

TEST_CASE("Single rule chained command construction. No recompile needed", "[rule_runner]") {}
