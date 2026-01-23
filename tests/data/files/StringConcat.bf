<Config> cfg {
    compiler = "clang++"
    compilation_flags = []
    link_flags = []
    default_rule = "app"
}

prefix = "src/"
suffix = ".cpp"
filename = prefix + "main" + suffix

base_name = "my_app"
full_name = base_name + "_v2"

<Rule> app {
    deps = [filename]
    step = Step::COMPILE
}
