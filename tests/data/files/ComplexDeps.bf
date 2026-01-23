<Config> cfg {
    compiler = "clang++"
    compilation_flags = ["-Wall", "-Wextra", "-std=c++20"]
    link_flags = ["-lpthread"]
    default_rule = "main_app"
}

main_sources = ["main.cpp", "app.cpp"]
lib_sources = ["utils.cpp", "helpers.cpp"]
all_sources = main_sources + lib_sources

main_objs = ["main", "app"]
lib_objs = ["utils", "helpers"]
all_objs = main_objs + lib_objs

<MultiRule> compile_main {
    deps = main_sources
    output = main_objs
    step = Step::COMPILE
}

<MultiRule> compile_lib {
    deps = lib_sources
    output = lib_objs
    step = Step::COMPILE
}

<Rule> main_app {
    deps = all_objs
    step = Step::LINK
}

<Clean> clean {
    targets = ["main_app"] + all_objs
}
