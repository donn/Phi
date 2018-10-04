# Draft: Lexemes

*tokens*:
* *keyword*
* *identifier*
* *numeric*
* *punctuator*

*keyword*:
* **module**
* **interface**
* **namespace**
* **if**
* **else**
* **switch**
* **mux**
* **case**
* **for**
* **while**
* **sync**
* **async**
* **@resetHigh**
* **@resetLow**
* **Var**
* **Wire**
* **Register**

*identifier*:
* *starter* *ender*

*ender*:
* *ender* *starter*
* *ender* *digit*
* ε

*starter*:
* **A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h i j k l m n o p q r s t u v w x y z _**

*numeric*:
* *decimal*
* *fixed-width-numeric*

*decimal*:
* *decimal* *digit*
* *digit*

*fixed-width-numeric*:
* *decimal* *separator* *state-number*

*state-number*: any of
* *state-number* *state-digit*
* *state-digit*

*separator*:
* **b o d x**

*state-digit*: any of
* **z** **x** *digit*

*digit*: any of
* **0 1 2 3 4 5 6 7 8 9**

*punctuator*: any of
* **@ $ ! ~ + - * < > / % | & ^ ? : ; , = . { } [ ] ( ) >= <= &+ &- &> &< &>= &<=**