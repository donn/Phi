# Code Style Manual
Note this is mostly for me: I'm very desperate for help with this thing and
will accept anything.

## Spacing and Indentation

Use 4 space indentation for anything but Makefiles, where you have to
use tabs. There is no maximum line length limit, but *please* don't be ridiculous.

We use the **Kernighan & Ritchie** style for **non-empty blocks**:

-   No line break before the opening brace.

-   Line break after the opening brace.

-   Line break before the closing brace.

-   Line break after the closing brace, only if that brace terminates a
    statement or terminates the body of a method, constructor, or named class.
    For example, there is no line break after the brace if it is followed by
    else or a comma.

No exception is made for functions. Braces are used with
if, else, for, do and while statements, even when the body is empty or
contains only a single statement.

An empty block or block-like construct may be in the K&R style.
Alternatively, it may be closed immediately after it is opened, with no
characters or line break in between ({}), unless it is part of a
multi-block statement (one that directly contains multiple blocks:
if/else or try/catch/finally).

## Spacing

Leave a space after **keywords**: if, switch, case, for, do and while.

Do *not* leave a space after **function names** and **compiler
constructs**. For compiler constructs, always use parentheses.

Do *not* leave spaces between parentheses and contents. That is ugly.

```
⭕️ if (flag)

⭕️ sizeof(buffer)

⭕️ printf("Execute Order %i", 66)

❌ if( flag )

❌ sizeof buffer
```

As a general rule, any variable modifiers are best stuck to the actual type
instead of the variable name. This rule is flexible given that's not how C/C++
evaluate them, but still.

```
⭕️ int& cores;

⭕️ std::string* name;

❌ int * address;

❌ char *strobe;
```

## Operators

Use one space around (on each side of) most binary and ternary
operators, such as any of these:

```
= + - < > * / % | & ^ <= >= == != ? : . -> >> <<
```

But no space after unary operators (or before in case of postfix
operators):

```
& * + - ~ ! ++ --
```

Avoid overloading operators as much as possible unless in specific cases
where it does make sense, like a matrix class. But appending to an
array, for example? Not so much.

Use parentheses for complex operations. Break expressions into
subexpressions where possible.

In languages like C, avoid implicit promotion and demotion: please cast
explicitly, even when changing between what should seem obvious such as
char and short or unsigned int and unsigned long long.

## Naming

Please be as descriptive as possible. Linus Torvalds calls C spartan,
but the Linux kernel codebase is an undocumented mess so maybe he is not
the best person to ask here.

For classes, structs, enums and namespaces, use **UpperCamelCase**.

For variables, properties and function and methods, use
**lowerCamelCase**. This includes static properties.

Constants will vary. If the constant is defined using const type name =
value; the constant should be treated as a variable or property.

For compiler definitions however (including macros), use
**BLOCK\_CAPITAL\_SNAKE\_CASE**. For enums in a global namespace, also
use **BLOCK\_CAPITAL\_SNAKE\_CASE**, but try to prefix them somehow,
i.e. **ENUM\_NAME\_THEN\_FRIENDLY\_NAME** works fine. In a non-global
namespace, use **lowerCamelCase**.

In the global namespace, case sensitivity should not be an issue. In
general, having a class Foo and a global constant FOO is not allowed.

In local namespaces, however, Class class is an encouraged and
celebrated tradition.

Any non-block capital snake\_case is reserved for the C++ STL.

## Typedefs

Use typedefs sparingly. Common typedefs for ints: i.e. int32, uint8,
etc. are already available for this project.

**DO NOT USE \#define for types.** Macros are okay though.

## Functions and Methods

The definition should look something like this, generally:


⭕️
```
type name(int a, char b) {
    return a + (int)b;
}
```

This, however, is undesirable.

❌

```
type
Name(int a, char b) {

return a + b; }
```

You may have the type names on their own line if they are longer than 24
characters, though you should avoid having types longer than 24
characters in the first place.

For prototypes, also keep the parameter names. It adds just a little bit
of easy documentation to the function.

```
⭕️ float divide(float divisor, int dividend); //Weird order!
Documentation helps!

❌ float divide(float, int);
```

**DO NOT PASS OBJECTS BY REFERENCE.**

To "pass by reference", pass an actual pointer:

```c++
⭕️

void addOne(int* target) {
    auto& dereferenced = *target;
    dereferenced += 1;
}

// [...]

int counter = 0;

addOne(&counter);
```

The problem is, when passing purely by reference, some ambiguity is introduced:

❌
```c++
void addOne(int& target) {
    target += 1;
}

// [...]

int counter = 0;

addOne(counter);
```

It is not apparent that the variable is being modified.

```c++
auto& set = *data->blocks[0].stuff->array;
```

The use of references in C++ is generally not a bad idea, as long as the
scoping is local as seen above. It makes accessing the vector much
easier, especially in a loop, and as the scope is limited locally, this
is much more readable.  

## Acknowledgements

This style guide is based chiefly on these sources.

-   Torvalds, Linus: Linux kernel coding style
    > [https://www.kernel.org/doc/html/v4.10/process/coding-style.html](https://www.kernel.org/doc/html/v4.10/process/coding-style.html)

-   Wenderlich, Ray: Official raywenderlich.com Swift Style Guide
    > [https://github.com/raywenderlich/swift-style-guide](https://github.com/raywenderlich/swift-style-guide)

-   Google, Inc: Java Style Guide
    > [https://google.github.io/styleguide/javaguide.html\#s4.1-braces](https://google.github.io/styleguide/javaguide.html#s4.1-braces)
