<Config> cfg {
    compiler = "g++"
    compilation_flags = ["-g", "-O2"]
    link_flags = ["-lm"]
    default_rule = "app"
}

src_files = ["main.cpp", "utils.cpp", "parser.cpp"]
obj_files = ["main", "utils", "parser"]

<MultiRule> compilation {
    deps = src_files
    output = obj_files
    step = Step::COMPILE
}

<Rule> app {
    deps = obj_files
    step = Step::LINK
}
