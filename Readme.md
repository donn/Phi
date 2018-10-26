# <image src="Assets/Logo/400.png" height="200px"/>
Phi is a hardware description language that aims to succeed Verilog using concepts from modern programming languages.

It is our goal to design a well-defined language with:
* No behavioral ambiguity
* Powerful meta-definition constructs
* A clear definition and meta-definition divide

One that is unsurprising to newcomers yet one that is close enough to Verilog to not be as shocking to veteran engineers as languages such as MyHDL and Scala.

# Dependencies and dependency guidelines
Phi requires a POSIX-compliant system and a competent C++17-compatible compiler.
* i.e. MinGW should be used on Windows and no GCC or Clang specific extensions can be used.

This also requires a recent of version of GNU Flex and GNU Bison. These generate code that is incompatible with C++17, so they are treated a little bit differently.

## Usage
### General: After Cloning
Run `git submodule update --init --recursive`.

We use open source libraries for various functions, and they're all submoduled.

### macOS
Install Xcode from the App Store.

To get a newer version of Flex/Bison, install [Homebrew](https://brew.sh) and then...

```bash
    brew install flex bison
```

You will need to add these to PATH, as brew won't.

### GNU/Linux
You'll need g++ and git, obviously.


Install Flex/Bison or some other Lex/Yacc compatible toolchain other using your software repository.

#### Debian-based OSes (incl. Ubuntu, elementary...)

```bash
    sudo apt install flex bison
```

# License
Phi is available under the Apache 2.0 license, available at the root of this project as 'License'.
