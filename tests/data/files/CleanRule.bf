<Config> cfg {
    compiler = "clang++"
    compilation_flags = []
    link_flags = []
    default_rule = "app"
}

<Rule> app {
    deps = ["main.cpp"]
    step = Step::LINK
}

<Clean> clean {
    targets = ["app", "main.o", "utils.o"]
}
