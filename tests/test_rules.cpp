#include <unordered_set>

#include "catch.hpp"
#include "mocks/mock_fs_gateway.hpp"
#include "src/built_in/enums.hpp"
#include "src/dictionaries/rules.hpp"

using namespace std::chrono_literals;

TEST_CASE("Single rule command construction", "[rule_runner]") {
    SingleRule rule{"prog", {"a.c", "b.c"}, Step::COMPILE, Location{0, 0, 0}};
    const Config cfg{"cfg", "clang++", {"-Werror", "-Wall"}, {"-DontInclude"}, "test"};
    const std::vector<Command> commands = rule.get_commands(cfg);

    REQUIRE(commands.size() == 1);
    const Command cmd = commands.at(0);

    // Certain command args can be in any order and others we must check
    // 'clang++' + '-Werror' + '-Wall' + ' a.c' + 'b.c' + '-o' + 'prog' -> size = 7
    REQUIRE(cmd.size() == 7);
    REQUIRE(cmd.at(0) == "clang++");

    std::unordered_set<std::string> got_arg_set(cmd.begin(), cmd.end());
    std::unordered_set<std::string> exp_arg_set = {"-Werror", "-Wall", "a.c", "b.c"};
    for (const std::string& arg : exp_arg_set) {
        REQUIRE(got_arg_set.contains(arg));
    }

    auto out_prefix = std::ranges::find(cmd, "-o");
    REQUIRE(out_prefix != cmd.end());
    out_prefix++;
    REQUIRE(out_prefix != cmd.end());
    REQUIRE(*out_prefix == "prog");
}

TEST_CASE("Single rule command construction with link step", "[rule_runner]") {
    SingleRule rule{"program", {"a.o", "b.o", "c.o"}, Step::LINK, Location{0, 0, 0}};
    const Config cfg{"cfg", "g++", {"-Werror", "-Wall"}, {"-lpthread", "-lm"}, "test"};
    const std::vector<Command> commands = rule.get_commands(cfg);

    REQUIRE(commands.size() == 1);
    const Command cmd = commands.at(0);

    // 'g++' + '-lpthread' + '-lm' + 'a.o' + 'b.o' + 'c.o' + '-o' + 'program' -> size = 8
    REQUIRE(cmd.size() == 8);
    REQUIRE(cmd.at(0) == "g++");

    std::unordered_set<std::string> got_arg_set(cmd.begin(), cmd.end());
    std::unordered_set<std::string> exp_arg_set = {"-lpthread", "-lm", "a.o", "b.o", "c.o"};
    for (const std::string& arg : exp_arg_set) {
        REQUIRE(got_arg_set.contains(arg));
    }

    // Verify link flags are used, not compilation flags
    REQUIRE_FALSE(got_arg_set.contains("-Werror"));
    REQUIRE_FALSE(got_arg_set.contains("-Wall"));

    auto out_prefix = std::ranges::find(cmd, "-o");
    REQUIRE(out_prefix != cmd.end());
    out_prefix++;
    REQUIRE(out_prefix != cmd.end());
    REQUIRE(*out_prefix == "program");
}

TEST_CASE("Multi rule command construction", "[rule_runner]") {
    std::vector<std::string> deps = {"file1.cpp", "file2.cpp", "file3.cpp"};
    std::vector<std::string> outputs = {"file1.o", "file2.o", "file3.o"};
    MultiRule rule{"compilation", deps, outputs, Step::COMPILE, Location{0, 0, 0}};
    const Config cfg{"cfg", "clang++", {"-std=c++20", "-O2"}, {"-lz"}, "test"};
    const std::vector<Command> commands = rule.get_commands(cfg);

    REQUIRE(commands.size() == 3);

    // Check each command corresponds to one input/output pair
    for (size_t i = 0; i < 3; i++) {
        const Command& cmd = commands.at(i);

        // 'clang++' + '-std=c++20' + '-O2' + 'fileX.cpp' + '-o' + 'fileX.o' -> size = 6
        REQUIRE(cmd.size() == 6);
        REQUIRE(cmd.at(0) == "clang++");

        std::unordered_set<std::string> got_arg_set(cmd.begin(), cmd.end());
        REQUIRE(got_arg_set.contains("-std=c++20"));
        REQUIRE(got_arg_set.contains("-O2"));
        REQUIRE(got_arg_set.contains(deps[i]));

        // Verify compilation flags are used, not link flags
        REQUIRE_FALSE(got_arg_set.contains("-lz"));

        auto out_prefix = std::ranges::find(cmd, "-o");
        REQUIRE(out_prefix != cmd.end());
        out_prefix++;
        REQUIRE(out_prefix != cmd.end());
        REQUIRE(*out_prefix == outputs[i]);
    }
}

TEST_CASE("Clean rule command construction", "[rule_runner]") {
    std::vector<std::string> targets = {"app", "file1.o", "file2.o", "temp.txt"};
    CleanRule rule{"clean", targets, Location{0, 0, 0}};
    const Config cfg{"cfg", "clang++", {}, {}, "test"};
    const std::vector<Command> commands = rule.get_commands(cfg);

    REQUIRE(commands.size() == 1);
    const Command cmd = commands.at(0);

    // 'rm' + 'app' + 'file1.o' + 'file2.o' + 'temp.txt' -> size = 5
    REQUIRE(cmd.size() == 5);
    REQUIRE(cmd.at(0) == "rm");

    std::unordered_set<std::string> got_arg_set(cmd.begin() + 1, cmd.end());
    for (const std::string& target : targets) {
        REQUIRE(got_arg_set.contains(target));
    }
}

TEST_CASE("Single rule should_run returns true when output doesn't exist", "[rule_runner]") {
    SingleRule rule{"prog", {"a.c", "b.c"}, Step::COMPILE, Location{0, 0, 0}};
    auto fs = std::make_unique<MockFsGateway>();

    // Dependencies exist but output does not
    fs->touch("a.c");
    fs->touch("b.c");

    REQUIRE(rule.should_run(*fs) == true);
}

TEST_CASE("Single rule should_run returns true when dependency is newer", "[rule_runner]") {
    SingleRule rule{"prog", {"a.c", "b.c"}, Step::COMPILE, Location{0, 0, 0}};
    MockFsGateway fs;

    auto old_time = std::filesystem::file_time_type::clock::now() - 10s;
    auto new_time = std::filesystem::file_time_type::clock::now();

    // Output exists at old time
    fs.touch_at("prog", old_time);

    // One dependency is older, one is newer
    fs.touch_at("a.c", old_time);
    fs.touch_at("b.c", new_time);

    REQUIRE(rule.should_run(fs) == true);
}

TEST_CASE("Single rule should_run returns false when all dependencies are older", "[rule_runner]") {
    SingleRule rule{"prog", {"a.c", "b.c"}, Step::COMPILE, Location{0, 0, 0}};
    MockFsGateway fs;

    auto old_time = std::filesystem::file_time_type::clock::now() - 10s;
    auto new_time = std::filesystem::file_time_type::clock::now();

    // Output exists at new time
    fs.touch_at("prog", new_time);

    // All dependencies are older
    fs.touch_at("a.c", old_time);
    fs.touch_at("b.c", old_time);

    REQUIRE(rule.should_run(fs) == false);
}

TEST_CASE("Multi rule should_run returns true when output doesn't exist", "[rule_runner]") {
    std::vector<std::string> deps = {"file1.cpp", "file2.cpp"};
    std::vector<std::string> outputs = {"file1.o", "file2.o"};
    MultiRule rule{"compilation", deps, outputs, Step::COMPILE, Location{0, 0, 0}};
    MockFsGateway fs;

    // Dependencies exist but outputs do not
    fs.touch("file1.cpp");
    fs.touch("file2.cpp");

    REQUIRE(rule.should_run(fs) == true);
}

TEST_CASE("Multi rule should_run returns true when any dependency is newer", "[rule_runner]") {
    std::vector<std::string> deps = {"file1.cpp", "file2.cpp"};
    std::vector<std::string> outputs = {"file1.o", "file2.o"};
    MultiRule rule{"compilation", deps, outputs, Step::COMPILE, Location{0, 0, 0}};
    MockFsGateway fs;

    using namespace std::chrono_literals;
    auto old_time = std::filesystem::file_time_type::clock::now() - 10s;
    auto new_time = std::filesystem::file_time_type::clock::now();

    // Note: For MultiRule, the "name" field is used for should_run check
    fs.touch_at("compilation", old_time);

    fs.touch_at("file1.cpp", old_time);
    fs.touch_at("file2.cpp", new_time);  // This one is newer

    REQUIRE(rule.should_run(fs) == true);
}

TEST_CASE("Clean rule should_run always returns true", "[rule_runner]") {
    std::vector<std::string> targets = {"app", "file1.o", "file2.o"};
    CleanRule rule{"clean", targets, Location{0, 0, 0}};
    MockFsGateway fs;

    // Clean rule should always run regardless of file state
    REQUIRE(rule.should_run(fs) == true);

    // Even when targets exist
    fs.touch("app");
    fs.touch("file1.o");
    fs.touch("file2.o");

    REQUIRE(rule.should_run(fs) == true);
}

TEST_CASE("Single rule with no dependencies should run when output missing", "[rule_runner]") {
    SingleRule rule{"prog", {}, Step::COMPILE, Location{0, 0, 0}};
    MockFsGateway fs;

    REQUIRE(rule.should_run(fs) == true);
}

TEST_CASE("Multi rule with link step uses link flags", "[rule_runner]") {
    std::vector<std::string> deps = {"a.o", "b.o"};
    std::vector<std::string> outputs = {"liba.so", "libb.so"};
    MultiRule rule{"libraries", deps, outputs, Step::LINK, Location{0, 0, 0}};
    const Config cfg{"cfg", "g++", {"-Wall"}, {"-shared", "-fPIC"}, "test"};
    const std::vector<Command> commands = rule.get_commands(cfg);

    REQUIRE(commands.size() == 2);

    for (size_t i = 0; i < 2; i++) {
        const Command& cmd = commands.at(i);

        std::unordered_set<std::string> got_arg_set(cmd.begin(), cmd.end());
        REQUIRE(got_arg_set.contains("-shared"));
        REQUIRE(got_arg_set.contains("-fPIC"));

        // Verify link flags are used, not compilation flags
        REQUIRE_FALSE(got_arg_set.contains("-Wall"));
    }
}