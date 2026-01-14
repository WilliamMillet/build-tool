- String quote escaping
- Cleaner error messages. Maybe show the actual line when throwing the error
- Look into better ways to throw exceptions without swallowing up stuff like exception type.
- Maybe I could have my own Python-like exception hierachy with name error, index error, etc
- Possibly change Value to be more polymorphic and more open closed
- Have a consume ovelroad that allows to consume a specific token, asserting that is consumed

- Enums expanded as macros effectively to ints
- Gotta read slightly more in depth into the whole parser thing. My current structure is shit
- This could be good? https://pgrandinetti.github.io/compilers/page/how-to-design-a-parser/

Test to do 
- Value. Test for a few types:
    - move constructor
    - copy constructor
    - move assignment operator
    - copy asignment operator destructor
    - addition