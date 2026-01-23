<Config> cfg {
    compiler = "gcc"
    compilation_flags = ["-g", "-Wall"]
    link_flags = []
    default_rule = "app"
}

# Test nested list concatenation
list_a = ["a", "b"]
list_b = ["c", "d"]
combined = list_a + list_b

nested = [["inner1", "inner2"], ["inner3"]]

<Rule> app {
    deps = combined
    step = Step::LINK
}
