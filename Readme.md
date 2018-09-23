# <image src="Assets/Logo/400.png" height="200px"/>
Phi is a hardware description language that aims to succeed Verilog using concepts from modern programming languages.

It is our goal to design a well-defined language with:
* No behavioral ambiguity
* Powerful meta-definition constructs
* A clear definition and meta-definition divide

One that is unsurprising to newcomers yet one that is close enough to Verilog to not be as shocking to veteran engineers as languages such as MyHDL and Scala.

# Targets
Phi should compile on all major desktop operating systems: Windows, macOS and GNU/Linux. 

All code written for this project should be be compiled with **c++14** and **pedantic** flags
* i.e. MinGW should be used on Windows and no GCC or Clang specific code should be used.

As inferable, Phi will be implemented in Lex, Yacc and C++.

By the end of June 2018, we hope to present a working prototype of a Verilog translator.

# License
Phi is available under the Apache 2.0 license, available at the root of this project under 'License'.
