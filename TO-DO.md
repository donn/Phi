# List of problems or limitations with the current compiler by difficulty
## Implementation
### Advanced
* Arrays are not functional
* For loops are not functional

### Grueling
* Parameter system is not functional

#### Exhaustiveness
* No driving guarantees in `comb` blocks
    * A user is not forced to make their switches exhaustive, make sure a wire is not a function of itself, et cetera
* No exhaustivity guarantees in `mux` statements

## Design
* Module system vs Preprocessor
* Idiomatic decoding of arrays of components