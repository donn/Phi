# <image src="Assets/Logo/400.png" height="200px"/>
Phi is a hardware description language that aims to succeed Verilog using concepts from modern programming languages.

It is our goal to design a well-defined language with:
* No behavioral ambiguity
* Powerful meta-definition constructs
* A clear definition and meta-definition divide

One that is unsurprising to newcomers yet one that is close enough to Verilog to not be as shocking to veteran engineers as languages such as MyHDL and Scala.

# Dependencies and dependency guidelines
Building Phi requires:

* Git
* A POSIX-compliant system
* A C++17 compiler that supports **standard** C++17
    * No GCC or Clang specific extensions can be used.
* GNU Flex
* GNU Bison
* Make
* Ruby
    * (For helper scripts)

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
Install git, gcc, make, flex and bison toolchain other using your software repository.

#### Debian-based OSes (incl. Ubuntu, elementary...)
Use apt.

```bash
    sudo apt-get install git gcc flex bison make ruby
```

### Windows
Get [MSYS2](https://www.msys2.org/).

In the MSYS2 terminal, invoke:

```bash
    pacman -Syu git gcc flex bison make ruby
```

# License
Phi is available under the Apache 2.0 license, available at the root of this project as 'License'.
