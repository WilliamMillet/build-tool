<Config> cfg {
    compiler = "clang++"
    compilation_flags = ["-g", "-Wall", "-Werror"]
    link_flags = []
    default_rule = "app" # This is the equivalent of 'all' from make
}

<Rule> app {
    deps = ["func.cpp", "main.cpp"]
    step = Step::COMPILE
}

<Clean> clean {
    targets = ["app"]
}