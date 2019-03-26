# Appendix B. Grammar

## Top Level

*description*:
* *declaration* *description*
* **namespace** *identifier* **{** *description* **}** *description*
* ε

*declaration*:
* **module** *identifier* *template-declaration* **(** *port-declaration-list* **)** *inheritance* *block*
* **interface** *identifier* *template-declaration* **(** *port-declaration-list* **)** *inheritance* **;**


## Ports

*port-declaration-list*:
* *populated-port-declaration-list*
* ε

*populated-port-declaration-list*:
* *identifier* **:** *optional-annotation* *port-polarity* *optional-bus-declaration* **,** *populated-port-declaration-list*
* *identifier* **:** *optional-annotation* *port-polarity* *optional-bus-declaration*

*port-polarity*:
* **Input**
* **Output**

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
* *lhexpression* **,** *inheritance-list*
* *lhexpression*

## Statements
*statement*:
* *optional-annotation* *subdeclaration* **;**
* *optional-annotation* *nondeclarative-statement* **;**
* *optional-annotation* *block-based*

*block-based*:
* *if*
* **for** *identifer* **in** *range* *block*
* **namespace** *identifier* *block*
* **switch** *expression* **{** *labeled-statement-list* **}**
* **switch** *fixed-width-special* **{** *labeled-statement-list* **}**
* **comb** *block*

*if*:
* **if** *expression* *block* *else*

*else*:
* **else** *if*
* **else** *block*
* ε

*labeled-statement-list*:
* **case** *expression* **:** *statement_list* *labeled_statement_list*
* **default** *expression* **:** *statement_list*
* ε

*block*:
* **{** statement-list **}**

*statement-list*:
* *statement* *statement_list* 
* ε

## Subdeclarations
*subdeclaration*:
* *dynamic-width* *optional-bus-declaration* *declaration-list*
* *expression* *optional-template* *identifier* *ports*

*dynamic-width*:
* **Var**
* **Wire** 
* **Register**
* **Latch**

*optional-bus-declaration*:
* **[** *range* **]**
* ε

*optional-array-declaration*:
* **[** *expression* **]**
* ε

*declaration-list*:
* *identifier* *optional-array-declaration* *optional-assignment* **,** *declaration-list*
* *identifier* *optional-array-declaration* *optional-assignment*

*optional-assignment*:
* **=** *expression*
* ε

*optional-template*:
* **<** *template-list* **>**
* ε

*template-list*:
* *identifier* **:** **(** *expression* **)** **,** *template-list*
* *identifier* **:** **(** *expression* **)**
* ε

*optional-ports*:
* *ports*
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

*labeled-statement-list*:
* **case** number **:** *statement-list* *labeled-statement-list*
* **default** **:** *statement-list*
* ε

## Expressions

*lhexpression*:
* *identifier*
* *lhexpression* **.** *lhexpression*
* *lhexpression* **[** *range* **]**
* *lhexpression* **[** *expression* **]**

*expression*:
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
* **[** *concatenation* **]**
* **$** *expression* **(** *procedural-call* **)**
* **(** *expression* **)**
* *fixed-width-numeric*
* *numeric*
* *lhexpression*

*range*:
* *expression* **..** *expression*

*concatenation*:
* *concatenable* **,** *concatenation*
* *concatenable*

*concatenatable*:
* *expression*
* *expression* **[[** *concatenation* **]** **]**

*mux*:
* **mux** *expression* *mux-block*
* **mux** *fixed-width-special* *mux-block*

*procedural-call*:
* *procedural-call-list*
* ε

*procedural-call-list*:
* *expression* **,** *procedural-call-list*
* *string* **,** *procedural-call-list*
* *expression*
* *string*

*mux-block*:
* **(** *labeled-expression-list* **)**

*labeled-expression-list*:
* **case** *expression* **:** *expression* **,** *labeled-expression-list*
* *expression* **:** *expression*
* **default** **:** *expression*
* ε
