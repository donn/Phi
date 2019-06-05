# Φ Phi
Phi is a hardware description language that aims to succeed Verilog using concepts from modern programming languages.

It is our goal to design a well-defined language with:
* No behavioral ambiguity
* A clear definition and meta-definition divide

It's basically a mix of Swift/C++'s syntax with Verilog's semantics.

Phi supports Linux, macOS, and Windows with MSYS2.

# Dependencies
## Running
* LLVM 8.0
* Ruby 2.3+

## Building
All running dependencies, plus:

* A POSIX environment
* A C++17 compiler that supports **standard** C++17
    * No GCC or Clang specific extensions are used. Any used by accident are a bug we are interested in fixing.
    * We still recommend Clang + LLDB for debugging.
* Git
* Make
* GNU Bison

### macOS
Install Xcode from the App Store.

For the other dependencies, we recommend [Homebrew](https://brew.sh). Install it using the command in the link provided, then invoke:

```bash
    brew install bison llvm
```

This will take some time, llvm is big.

You will need to add bison to PATH, as brew won't. Do this however you want: a suggestion is to add this to your shell's profile:
```sh
export BISON="/usr/local/opt/bison/bin/bison"
```

You will also need to expose LLVM Config to the compiler, as brew *also* won't. Again, it is suggested to add this to your shell's profile:
```sh
export LLVM_CONFIG="/usr/local/opt/llvm/bin/llvm-config"
```

(For the default shells, that's `~/.bash_profile` on Mojave and below, and `~/.zprofile` on Catalina and above.)

### GNU/Linux
Install git, gcc, make and bison using your package manager.

If you have Clang and you want to use it, you can export and set the `CC` and `CXX` environment variables to `clang` and `clang++` respectively.

#### Debian-based OSes (incl. Ubuntu, elementary...)
Using apt...

```sh
    sudo apt-get install git build-essential bison llvm ruby-dev
```

If you're into clang...

```sh
    sudo apt-get install clang lldb
```

### Windows with MSYS2
First, get [MSYS2-x86_64](https://www.msys2.org/) if you haven't already.

```sh
pacman -S git make mingw-w64-x86_64-gcc bison mingw-w64-x86_64-llvm ruby
```

You can also use Clang and lldb if you're into that:

```sh
pacman -S mingw-w64-x86_64-clang lldb
```

We are interested in supporting Visual Studio in the future.

# Build Instructions
Run `git submodule update --init --recursive`.

We use open source libraries for various functions, and they're all imported using git submodules.

You can then invoke either `make` or `make release`. The former produces a debug binary, which is slower but packs more features. It is more suitable for doing dev work on the actual compiler. If you intend on using the Phi compiler itself, we recommend `make release`.

# Usage Instructions
There are two invocation options here: `./phic` and `./phi`.

`./phic` is the actual compiler for Phi. It's a plain binary. You can write `./phi --help` for more information, but the short story is, to invoke it write `./phi <phi file name>`. If there are no errors returned, it creates a file with the same name as the filename given with ".sv" appended.

The former is a ruby script with certain capabilities including the ability to load .sv files and simulate them instantly: bypassing the .out file as a middle step. It also processes a special extension to SystemVerilog, ``` `phi ```. Tick phi allows you to include Phi files from SystemVerilog files, where the script will automatically invoke the Phi compiler on them.

To invoke it, simply write `./phi <.phi or .sv file>`. In case of a phi file, it acts like the compiler itself, but some options (especially the debug only ones) are not available.

Both executables comply with BSD-style system exits, and you can write `./phi(c) --help` for more info.

# ⚖️ License
Phi is available under the Apache 2.0 license, available at the root of this project as 'License'.

Please try to keep any copyleft code out of the final binary, and that includes LGPL libraries.
