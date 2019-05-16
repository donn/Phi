# List of problems or limitations with the current compiler by difficulty
## Implementation
### Intermediate
* Interfaces are not implemented
* System procedural call implementation unfinished
    * Error reporting is missing for the fromFile calls
    * $Sys.abs implementation only works accurately up to 52-bits, does not support negative numbers
* Implement & and | unary operators
* Assigning to enable without annotation does not yet work

### Advanced
* Namespace translation is often incorrect
    * Might require LHExpression encapsulation: would actually clean up a lot of the compiler's design
* Error reporting for semantics does not show location
* For loops and arrays are not functional

### Core
* Parameter system is not functional
* No driving guarantees in `comb` blocks
    * A user is not forced to make their switches exhaustive, make sure a wire is not a function of itself, et cetera
* No exhaustivity guarantees in `mux` statements

## Design
* Module system vs Preprocessor
* Idiomatic decoding of arrays of components