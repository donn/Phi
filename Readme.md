# <image src="Assets/Logo/400.png" height="200px"/>
Phi is a hardware description language that aims to succeed Verilog using concepts from modern programming languages.

It is our goal to design a well-defined language with:
* No behavioral ambiguity
* Powerful meta-definition constructs
* A clear definition and meta-definition divide

One that is unsurprising to newcomers yet one that is close enough to Verilog to not be as shocking to veteran engineers as languages such as MyHDL and Scala.

# Dependencies and dependency guidelines
Phi requires a POSIX-compliant system and a competent C++17-compatible compiler. The ultimate goal is portability between different operating systems and Lex/Yacc tools.

All C++ code written for this project should be be compiled with **c++14** and **pedantic** flags.
* i.e. MinGW should be used on Windows and no GCC or Clang specific extensions can be used.

## Usage
### General: After Cloning
Run `git submodule update --init --recursive`.

We use open source libraries for various functions, and they're all submoduled.

### macOS
Install Xcode from the App Store. At the time of writing, you do not need a later version of bison than the one that is provided with Xcode.

### GNU/Linux
Install Flex/Bison or some other Lex/Yacc compatible toolchain other using your software repository.

### Windows
To-do.

# License
Phi is available under the Apache 2.0 license, available at the root of this project under 'License'.
