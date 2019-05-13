# Appendix A. Lexemes

These are the lexemes or tokens for the Phi Hardware Description Language.

These cannot be separated by spaces.

*tokens*:
* *keyword*
* *annotation*
* *identifier-token*
* *string*
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
* **comb**
* **Var**
* **Wire**
* **Register**

*annotation*:
* *@* *starter* *endsequence*

*identifier-token*:
* *starter* *endsequence*
* **\`** *starter* *endsequence* **\`**

*endsequence*:
* *ender* *ender*
* ε

*ender*:
* *starter*
* *decimal digit*
* in unicode ranges U+0300–U+036F, U+1DC0–U+1DFF, U+20D0–U+20FF, or U+FE20–U+FE2F

*starter*: any of
* **A B C D E F G H I J K L M N O P Q R S T U V W X Y Z a b c d e f g h i j k l m n o p q r s t u v w x y z _**
* in unicode ranges U+00A8, U+00AA, U+00AD, U+00AF, U+00B2–U+00B5, or U+00B7–U+00BA
* in unicode ranges U+00BC–U+00BE, U+00C0–U+00D6, U+00D8–U+00F6, or U+00F8–U+00FF
* in unicode ranges U+0100–U+02FF, U+0370–U+167F, U+1681–U+180D, or U+180F–U+1DBF
* in unicode ranges U+1E00–U+1FFF
* in unicode ranges U+200B–U+200D, U+202A–U+202E, U+203F–U+2040, U+2054, or U+2060–U+206F
* in unicode ranges U+2070–U+20CF, U+2100–U+218F, U+2460–U+24FF, or U+2776–U+2793
* in unicode ranges U+2C00–U+2DFF or U+2E80–U+2FFF
* in unicode ranges U+3004–U+3007, U+3021–U+302F, U+3031–U+303F, or U+3040–U+D7FF
* in unicode ranges U+F900–U+FD3D, U+FD40–U+FDCF, U+FDF0–U+FE1F, or U+FE30–U+FE44
* in unicode ranges U+FE47–U+FFFD
* in unicode ranges U+10000–U+1FFFD, U+20000–U+2FFFD, U+30000–U+3FFFD, or U+40000–U+4FFFD
* in unicode ranges U+50000–U+5FFFD, U+60000–U+6FFFD, U+70000–U+7FFFD, or U+80000–U+8FFFD
* in unicode ranges U+90000–U+9FFFD, U+A0000–U+AFFFD, U+B0000–U+BFFFD, or U+C0000–U+CFFFD
* in unicode ranges U+D0000–U+DFFFD or U+E0000–U+EFFFD

*string*:
* **"** character-sequence **"**

*character-sequence*:
* *character* *character-sequence*
* *character*

*character*:
* **\** **"**
* Any valid Unicode character excluding U+0000-U+001F (control characters) and U+0022 (")

*numeric*:
* *decimal*
* *fixed-width-numeric*

*decimal*:
* *decimal* *decimal-digit*
* *decimal-digit*

*fixed-width-numeric*:
* *decimal* *separator* *state-number*
> if *state-number* contains **?**, the token *fixed-width-special* is created, otherwise, the token *fixed-width-numeric* is created

*state-number*: any of
* *state-number* *state-digit*
* *state-digit*

*separator*:
* **b o d x**

*state-digit*: any of
* **?** *digit*

*digit*: any of
*  **A B C D E F** *decimal-digit*

*decimal-digit*: any of
* **0 1 2 3 4 5 6 7 8 9**

*punctuator*: any of
* **$ ! ~ + - * < > / % | & ^ : ; , = . {{ { } [ ] ( ) >= <= &+ &- &> &< &>= &<=**