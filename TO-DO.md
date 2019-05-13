# List of limitations with the current compiler
* Error reporting for semantics does not show location
* Lack of a preprocessor/module system
* Parameter system is non-functional
* For loops and arrays do not work
* No driving guarantees in `comb` blocks
    * A user is not forced to make their switches exhaustive, make sure a wire is not a function of itself, et cetera
* Assigning to enable without annotation does not yet work
* Interfaces are not implemented
* Namespace translation is finicky
* Syscall implementation gotchas
    * Error reporting is missing for the fromFile calls
    * $Sys.abs implementation only works accurately up to 52-bits, does not support negative numbers which makes it pretty useless