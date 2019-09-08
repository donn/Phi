# List of problems or limitations with the current compiler by difficulty
## Implementation
### Intermediate
* Interfaces are not implemented
* Assigning to enable without annotation does not yet work

### Advanced
* For loops and arrays are not functional

### Core
* Parameter system is not functional
* No driving guarantees in `comb` blocks
    * A user is not forced to make their switches exhaustive, make sure a wire is not a function of itself, et cetera
* No exhaustivity guarantees in `mux` statements

## Design
* Module system vs Preprocessor
* Idiomatic decoding of arrays of components