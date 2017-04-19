[+ AutoGen5 template h +]
[+ FOR enum +]
typedef enum {[+
   FOR value "," +]
    [+ prefix +]_[+ (get "value") +][+
   ENDFOR value +] }  [+name+];
[+ ENDFOR enum +]
