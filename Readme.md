# <image src="Assets/Logo/400.png" height="200px"/>
Phi is a hardware description language that aims to succeed Verilog using concepts from modern programming languages.

It is our goal to design a well-defined language with:
* No behavioral ambiguity
* A clear definition and meta-definition divide

It's basically a mix of Swift/C++'s syntax with Verilog's semantics.

# Dependencies
## Running
* LLVM 
* Ruby 2.3+ (optional, for main script)

## Build
All running dependencies, plus:

* A POSIX environment
* A C++17 compiler that supports **standard** C++17
    * No GCC or Clang specific extensions are used. Any used by accident are a bug we are interested in fixing.
* Git
* Make
* GNU Bison

## Usage
### All: After Cloning
Run `git submodule update --init --recursive`.

We use open source libraries for various functions, and they're all imported using git submodules.

### macOS
Install Xcode from the App Store.

For the other dependencies, we recommend [Homebrew](https://brew.sh). Install it using the command in the link provided, then invoke:

```bash
    brew install bison llvm
```

This will take some time, llvm is big.

You will need to add bison to PATH, as brew won't. Do this however you want: a recommended setup is to add this to `~/.bash_profile` (or your shell's equivalent):
```sh
export PATH="/usr/local/opt/bison/bin:$PATH"
```

You also need to expose LLVM to the compiler, as brew also won't. The Makefile supports $LDFLAGS and $CPPFLAGS, so you can just add these to your `~/.bash_profile` too:
```sh
export LDFLAGS="-L/usr/local/opt/llvm/lib"
export CPPFLAGS="-I/usr/local/opt/llvm/include"
```

### GNU/Linux
Install git, gcc, make and buson using your package manager.

#### Debian-based OSes (incl. Ubuntu, elementary...)
Use apt.

```bash
    sudo apt-get install build-essential bison llvm
```

# License
Phi is available under the Apache 2.0 license, available at the root of this project as 'License'.

Please try to keep any copyleft code out of the final binary, and that includes LGPL libraries.