<Config> cfg {
    compiler = "clang++"
    compilation_flags = ["-g", "-Wall"]
    link_flags = []
    default = "app" # This is the equivalent of 'all' from make
}

<MultiRule> compilation {
    deps = cpp_files
    output = cpp_names
    step = Step::Compile
}