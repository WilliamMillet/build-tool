# This file contains cyclical dependencies and should cause an error
<Config> cfg {
    compiler = "clang++"
    compilation_flags = []
    link_flags = []
    default_rule = "a"
}

# a depends on b, b depends on a - cyclical!
a = b + "foo"
b = a + "bar"

<Rule> my_rule {
    deps = [a]
    step = Step::COMPILE
}
