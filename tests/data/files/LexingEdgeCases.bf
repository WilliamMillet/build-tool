# Syntax will not work but should be lexed correctly
cpp_files = files(src_dir)
<Clean> clean {
    remove = ["app", ["nested", [[[]]]], ] + file_names(cpp_files)
    {
        {
            {
                {}
            }
        }
    }
}