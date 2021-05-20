# Φ Phi (Alpha)
![macOS CI](https://github.com/donn/Phi/actions/workflows/mac.yml/badge.svg)
![Linux CI](https://github.com/donn/Phi/actions/workflows/linux.yml/badge.svg)

Phi is a pure-ish RTL hardware description language based on Verilog that aims to be safer and more explicit.

Phi aims to have:
* No behavioral ambiguity
* A clear definition and meta-definition divide

Syntactically, it takes a lot from C/C++/Swift. Semantically, it takes the best parts of Verilog.

Phi supports Linux, macOS, and Windows with MSYS2.

# Examples
## Counter
```phi
module Counter(
    clock: Input,
    reset: Input,
    enable: Input,
    output: Output[31..0]
) {
    Register[31..0] counter = 32b0
    counter.clock = clock
    counter.reset = reset
    counter.data = counter &+ 32b1
    counter.enable = enable

    output = counter
}
```

# Dependencies
## Running
* A reasonably recent libstdc++.

## Building
* A POSIX environment
* A C++17 compiler that supports **standard** C++17
    * No GCC or Clang specific extensions are used. Any used by accident are a bug we are interested in fixing.
    * We still recommend Clang + LLDB for debugging.
* Git (for submodules)
* GNU Make 3.8.1+
* GNU Bison 3.0.4+
* LLVM 6.0+
* Ruby 2.3+
* zlib

### macOS
Install Xcode 9.0 or higher from the App Store.

For the other dependencies, we recommend [Homebrew](https://brew.sh). Install it using the command in the link provided, then invoke:

```bash
    brew install ruby bison llvm
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
Phi has been tested and is working on Ubuntu 18.04, 19.04 and their derivatives.

Using apt...

```sh
    sudo apt-get install -y build-essential git make llvm-11-dev zlib1g-dev bison ruby
```

If you're into clang...

```sh
    sudo apt-get install clang lldb
```

Do note you'll need to expose LLVM Config to the compiler- it is suggested to add this to your shell's profile.
```sh
export LLVM_CONFIG="llvm-config-11"
```

### Windows with MSYS2
First, get [MSYS2-x86_64](https://www.msys2.org/) if you haven't already.

```sh
pacman -S git make mingw-w64-x86_64-gcc bison mingw-w64-x86_64-llvm ruby
```

You can still also use Clang and lldb if you're into that:

```sh
pacman -S mingw-w64-x86_64-clang lldb
```

We are interested in supporting Visual Studio in the future.

# Build Instructions
Run `git submodule update --init --recursive --depth 1`.

We use open source libraries for various functions, and they're all imported using git submodules.

You can then invoke either `make` or `make release`. The former produces a debug binary, which is slower but packs more features. It is more suitable for doing dev work on the actual compiler. If you intend on using the Phi compiler itself, we recommend `make release`.

# Usage Instructions
`./phic` can be used to invoke Phi. It's a plain binary. You can write `./phi --help` for more information, but the short story is, to invoke it write `./phi <phi source file>`. Currently just the one is supported. If there are no errors returned, it creates a file with the same name as the filename given with ".sv" appended.

# ⚖️ Copyright & Licensing
©2019-2021 The Phi Authors. Phi is available under the Apache 2.0 license, available at the root of this project as 'License'.

Phi was partially developed as part of senior project coursework at the American University in Cairo.

# Special Thanks
* [Prof. Shalan](https://github.com/shalan) for consistent input and encouragement.
* [Genivia/RE-flex](https://github.com/Genivia/RE-flex), our primary dependency, for not breaking the API and the CLI every three seconds like some libraries do.