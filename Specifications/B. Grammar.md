# Draft: Grammar

*description*:
* *declaration* *description*
* ε

*declaration*:
* **module** *identifier* *template_declaration* **(** *port_declaration_list* **)** *inheritance* *block*
* **interface** *identifier* *template_declaration* **(** *port_declaration_list* **)** *inheritance* **;**

*port_declaration_list*:
* *populated_port_declaration_list*
* ε

*populated_port_declaration_list*:
* *identifier* **:** *lhexpression* **,** *populated_port_declaration_list*
* *identifier* **:** *lhexpression*

*port_declaration*:
* *port_polarity*
* **@resetHigh** *port_polarity*
* **@resetLow** *port_polarity*

*port_polarity*:
* **input**
* **output**
* **input** *array_subscript*
* **output** *array_subscript*

*template_declaration*:
* **<** *template_declaration_list* **>**

*template_declaration_list*:
* *identifier* *optional_template_assignment* *template_declaration_list*
* *identifier* *optional_template_assignment*

*optional_template_assignment*:
* **=** (*expression*)
* ε


*inheritance*:
* **:** *inheritance_list*
* ε

*inheritance_list*:
* *lh_expression* **,** *inheritance_list*
* *lh_expression*

*block*:
* **{** statement_list **}**

*statement_list*:
* *statement_list* *statement*
* ε

*statement*:
* *subdeclaration* **;**
* *nondeclarative_statement* **;**
* *block_based*

*block_based*:
* **namespace** *identifier* *block*
* **@** *expression* *block*
* **async** *block*
* *if*

*if*:
* **if** *expression* *block* *else*

*else*:
* **else** *if*
* **else** *expression* *block*
* ε

*subdeclaration*:
* *subscriptable_dynamic_width* *declaration_list*
* *probable_template* *identifier* *ports*

*subscriptable_dynamic_width*:
* *dynamic_width*
* *dynamic_width* *array_subscript*

*dynamic_width*:
* **Var**
* **Wire** 
* **Register**

*declaration_list*:
* lhexpression optional_assignment **,** declaration_list
* lhexpression optional_assignment

*optional_assignment*:
* **=** *expression*
* ε

*probable_template*:
* *lhexpression*
* *lhexpression* **<** *template_list* **>**

*template_list*:
* *identifier* **:** **(** *expression* **)** **,** template_list
* *identifier* **:** **(** *expression* **)**
* ε;

*ports*:
* **(** **)**
* **(** port_list **)**

*port_list*:
* *identifier* **:** *expression* **,** port_list
* *identifier* **:** *expression*

*nondeclarative_statement*:
* lhexpression **=** *expression*

*switch_block*:
* **{** *labeled_statement_list* **}**

*labeled_statement_list*:
* **case** number **:** *statement_list* *labeled_statement_list*
* **default** **:** *statement_list*
* ε

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
* *expression* **..** *expression*
* **&** *expression*
* **|** *expression*
* **~** *expression*
* **[** *concatenation* **]**
* *lhexpression*
* **mux** *expression* *mux_block*
* **$** *identifier* **(** *procedural_call* **)**
* **(** *expression* **)**
* *number*

*mux_block*:
* **{** *labeled_expression_list* **}**

*labeled_expression_list*:
* **case** *number* **:** *expression* **;** *labeled_expression_list*
* **default** **:** *expression* **;**
* ε

*concatenation*:
* *expression* **,** concatenation
* *expression* **[** *expression* **]**
* *expression*


*procedural_call*:
* *procedural_call_list*
* ε

*procedural_call_list*:
* *expression* **,** *procedural_call_list*
* *expression*


*lhexpression*:
* *lhexpression* **.** *lhexpression*
* *lhexpression* **[** *expression* **]**
* *identifier*

*array_subscript*:
* **[** *expression* **]**


*number*:
* *decimal*
* *state-digit*
