<Config> cfg {
    compiler = "clang++"
    compilation_flags = ["-g", "-Wall"]
    link_flags = []
    default_rule = "app" # This is the equivalent of 'all' from make
}

<Rule> compilation {
    deps = ["a", "b"]
    step = Step::COMPILE
}