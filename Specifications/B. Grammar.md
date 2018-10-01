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

*switch_block*:
* **{** *labeled_statement_list* **}**

*labeled_statement_list*:
* **case** number **:** *statement_list* *labeled_statement_list*
* **default** **:** *statement_list*
* ε

*statement*:
* *subdeclaration* **;**
* *nondeclarative_statement* **;**
* **namespace** *identifier* *block*
* **@** *expression* *block*
* **if** *expression* *block*
* **switch** *expression* *switch_block*
* **mux** *expression* *switch_block*

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
* **$** *identifier* **(** *procedural_call* **)**
* **(** *expression* **)**
* *number*

*concatenation*:
* *expression* **,** concatenation
* *expression* **{** *expression* **}**
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
