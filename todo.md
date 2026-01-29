# Short term ideas
- Move all variables for classes to private for encapsulatory purposes
- Maybe implement a toString method for Value to simplify debugging
- Implement an equivalent of make -j. Apparently topological sort is useful for this
- Right now, MultiRule compiles everything, even if only one command needs running. Maybe fix this
- Make errors colourful. Maybe use a library for this. Could be a good opportunity for this

# Long-term ideas
- Have a cache of the file data. If the build file didnt change between runs, I can just used the cache stuff