# Appendix B. Grammar

## Top Level

*description*:
* *declaration* *description*
* **namespace** *identifier* **{** *description* **}**
* ε

*declaration*:
* **module** *identifier* *template-declaration* **(** *port-declaration-list* **)** *inheritance* *block*
* **interface** *identifier* *template-declaration* **(** *port-declaration-list* **)** *inheritance* **;**


## Ports

*port-declaration-list*:
* *populated-port-declaration-list*
* ε

*populated-port-declaration-list*:
* *identifier* **:** *expression* **,** *populated-port-declaration-list*
* *identifier* **:** *expression*

*port-declaration*:
* *port-polarity*
* *annotation* *port-polarity*

*port-polarity*:
* **Input** *optional-array-subscript*
* **Output** *optional-array-subscript*

## Templating
*template-declaration*:
* **<** *template-declaration-list* **>**

*template-declaration-list*:
* *identifier* *optional-template-assignment* *template-declaration-list*
* *identifier* *optional-template-assignment*

*optional-template-assignment*:
* **=** **(** *expression* **)**
* ε

## Inheritance
*inheritance*:
* **:** *inheritance-list*
* ε

*inheritance-list*:
* *lh-expression* **,** *inheritance-list*
* *lh-expression*

## Statements
*statement*:
* *optional-annotation* *subdeclaration* **;**
* *optional-annotation* *nondeclarative-statement* **;**
* *optional-annotation* *block-based*

*block-based*:
* *if*
* **for** *identifer* **in** *range* *block*
* **namespace** *identifier* *block*
* **switch** *expression* *switch-block*
* **async** *block*

*if*:
* **if** *expression* *block* *else*

*else*:
* **else** *if*
* **else** *block*
* ε

*switch-block*:
* **{** *labeled-statement-list* **}**

*labeled-statement-list*:
* **case** *expression* **:** *statement_list* *labeled_statement_list*
* **default** *expression* **:** *statement_list*
* ε

*block*:
* **{** statement-list **}**

*statement-list*:
* *statement_list* *statement*
* ε

## Subdeclarations
*subdeclaration*:
* *dynamic-width* *optional-bus-declaration* *declaration-list*
* *probable-template* *identifier* *ports*

*dynamic-width*:
* **Var**
* **Wire** 
* **Register**

*optional-bus-declaration*:
* **[** *range* **]**
* ε

*optional-array-declaration*:
* **[** *expression* **]**
* ε

*optional-ports*:
* *ports*
* ε

*declaration-list*:
* *identifier* *optional-array-declaration* *optional-assignment* **,** *declaration-list*
* *identifier* *optional-array-declaration* *optional-assignment*

*optional-assignment*:
* **=** *expression*
* ε

*probable-template*:
* *expression*
* *expression* **<** *template-list* **>**

*template-list*:
* *identifier* **:** **(** *expression* **)** **,** *template-list*
* *identifier* **:** **(** *expression* **)**
* ε

*ports*:
* **(** **)**
* **(** *port-list* **)**

*port-list*:
* *identifier* **:** *expression* **,** *port-list*
* *identifier* **:** *expression*

## Nondeclarative Statements

*nondeclarative-statement*:
* *expression* **=** *expression*
* *expression* *ports*

*switch-block*:
* **{** *labeled-statement-list* **}**

*labeled-statement-list*:
* **case** number **:** *statement-list* *labeled-statement-list*
* **default** **:** *statement-list*
* ε

## Expressions

*expression*:
* *expression* **?** *expression* **:** *expression*
* *expression* **||** *expression*
* *expression* **&&** *expression*
* *expression* **==** *expression*
* *expression* **!=** *expression*
* *expression* **>** *expression*
* *expression* **<** *expression*
* *expression* **>=** *expression*
* *expression* **<=** *expression*
* *expression* **<=** *expression*
* *expression* **>=** *expression*
* *expression* **&<=** *expression*
* *expression* **&>=** *expression*
* *expression* **+** *expression*
* *expression* **-** *expression*
* *expression* **&+** *expression*
* *expression* **&-** *expression*
* *expression* **|** *expression*
* *expression* **&** *expression*
* *expression* **^** *expression*
* *expression* **\*** *expression*
* *expression* **/** *expression*
* *expression* **%** *expression*
* *expression* **<<** *expression*
* *expression* **>>>** *expression*
* *expression* **>>** *expression*
* **&** *expression*
* **|** *expression*
* **~** *expression*
* *expression* **.** *expression*
* *expression* *subscript*
* **[** *concatenation* **]**
* **mux** *expression* *mux-block*
* **$** *identifier* **(** *procedural-call* **)**
* **(** *expression* **)**
* *identifier*
* *number*

* *subscript*:
* **[** *range* **]**
* **[** *expression* **]**

*range*:
* *expression* **..** *expression*

*mux-block*:
* **{** *labeled-expression-list* **}**

*labeled-expression-list*:
* **case** *expression* **:** *expression* **,** *labeled-expression-list*
* *expression* **:** *expression*
* **default** **:** *expression*
* ε

*concatenation*:
* *concatenable* **,** *concatenation*
* *concatenable*

*concatenatable*:
* *expression*
* *expression* **[[** *expression* *]* *]*

*procedural-call*:
* *procedural-call-list*
* ε

*procedural-call-list*:
* *expression* **,** *procedural-call-list*
* *expression*

*number*:
* *decimal*
* *state-digit*
