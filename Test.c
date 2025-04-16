/*
\n (0x0A): move cursor down one line

\r (0x0D): return to column 0 of current line

\b (0x08): backspace

\f (0x0C): form feed (clears screen on some terminals)

\a (0x07): bell
*/

//=====================Type Defs=====================

typedef unsigned char               byte;

typedef unsigned char      uint8_t;
typedef signed char        int8_t;

typedef unsigned short     uint16_t;
typedef signed short       int16_t;

typedef unsigned int       uint32_t;
typedef signed int         int32_t;

typedef struct time{
    byte hour;
    byte minute;
    byte second;
}time;

//===================================================

//====================Prototypes=====================

unsigned int get_uptime(void);
time get_time(void);
short strlen(const char* str);
byte strcmp(const char* str1, const char* str2);
void strcpy(const char* src, char* dest);
char get_key(void);
char get_scan(void);
void pchar(char c);
void get_str(uint16_t max, char* sto);
void pstr(const char* str);
void pint(int num);
void newline(void);
void clrscrn(void);
void mvcursor_up(void);
void mvcursor_down(void);
void mvcursor_right(void);
void mvcursor_left(void);
void mvcursor(short difx, short dify);
byte bcdtoi(byte bcd);
void uitos(int num, char* output);
int intpow(int base, short exp);

//===================================================

//no more bytes can be before this as they would be ran first (only compiler commands[#defines, #pragmas, etc.] and prototypes)
void start(){
    clrscrn();
    char input[256];
    while(1){
        pstr(">>> ");
        for(int i = 0; i < 255;i++){
            input[i] = '\0';
        }
        get_str(255, input);
        if(strcmp("halt", input) == 0){
            asm("hlt":::);
            while(1);
        }else if(strcmp("clear", input) == 0){
            clrscrn();
            continue;
        }else if(strcmp("uptime", input) == 0){
            pint(get_uptime());
        }else if(strcmp("getscan", input) == 0){
            pstr("scan code: ");
            pint((int)get_scan());
        }
        newline();
    }
}

//========================Time=======================

//gets uptime from boot
unsigned int get_uptime(void) {
    unsigned short high, low;
    unsigned short ticks;

    asm volatile (
        "movb $0x00, %%ah\n"
        "int $0x1A\n"
        "movw %%cx, %0\n"
        "movw %%dx, %1\n"
        : "=m"(high), "=m"(low)
        :
        : "ah", "cx", "dx"
    );

    ticks = ((unsigned int)high << 16) | low;

    return ticks / 18;  // rough approximation
}


//gets the current time (WIP)
time get_time(void){
    time tim;
    byte h, m, s;

    asm volatile (
        "movb $0x2, %%ah\n"
        "int $0x1A\n"
        "movb %%ch, %0\n"
        "movb %%cl, %1\n"
        "movb %%dh, %2\n"
        : "=m"(h), "=m"(m), "=m"(s)
        :
        : "ah", "ch", "cl", "dh"
    );

    tim.hour   = bcdtoi(h);
    tim.minute = bcdtoi(m);
    tim.second = bcdtoi(s);

    return tim;
}

//====================================================

//====================String Funcs===================

//returns the lengths of the given string
short strlen(const char* str){
    short len;
    for(len = 0; str[len] != '\0';len++);
    return len;
}

//returns 0 if they are the same
byte strcmp(const char* str1, const char* str2){
    if(strlen(str1) != strlen(str2)){
        return 1;
    }
    for(short i = 0; i < strlen(str1); i++){
        if(str1[i] != str2[i]){
            return 2;
        }
    }
    return 0;
}

//copies the first string into the second string
void strcpy(const char* src, char* dest){
    for(int i = 0; i < strlen(src); i++){       //if necisarry then also add return the amount of bytes copied
        dest[i] = src[i];
    }
}

//====================================================

//=========================IO=========================

//gets the current key being pressed
char get_key(void){
    char key;
    asm volatile (
        "movb $0x00, %%ah\n"
        "int $0x16\n"
        "movb %%al, %0\n"
        : "=r"(key)        // output: %0 gets value from AL
        :                  // no input
        : "ah", "al"       // clobbered: we're modifying AH and AL
    );
    return key;
    
}

//gets the scan code from the current key
char get_scan(void){
    char scan;
    asm volatile (
        "movb $0x00, %%ah\n"
        "int $0x16\n"
        "movb %%ah, %0\n"
        : "=r"(scan)        // output: %0 gets value from AH
        :                   // no input
        : "ah"              // clobbered: we're modifying AH
    );
    return scan;
}

//returns the string the user inputed before pressing enter (only reads up to MAX amount of characters)
void get_str(uint16_t max, char* sto){
    char cha;
    //char scan;
    for(int cur = 0; cur < max;cur++){
        cha = get_key();
        //scan = get_scan();
        if(cha == '\r'){
            break;
        }
        sto[cur] = cha;
        pchar(cha);
    }
    newline();
}

//prints the character held by c
void pchar(char c) {
    asm volatile (
        "movb $0x0E, %%ah\n"
        "movb %0, %%al\n"
        "int $0x10\n"
        :
        : "r"(c)
        : "ah", "al"
    );
}

//prints and entire string (not advised to use a newline character in this)
void pstr(const char* str){
    while (*str) {
        pchar(*str++);
    }
}

//prints an integer
void pint(int num){
    char string[255];
    uitos(num, string);
    pstr(string);
}

//prints a newline
void newline(void){
    //pstr("\n\r"); // \n moves it down and \r returns it to the left
    pchar('\n');
    pchar('\r');
}

//====================================================

//=======================Cursor=======================

//clears the screen
void clrscrn(void) {
    //get rid of all pixels (set all to black)
    asm volatile (
        "movb $0x06, %%ah\n"
        "movb $0x00, %%al\n"
        "movb $0x07, %%bh\n"
        "movw $0x0000, %%cx\n"
        "movw $0x184F, %%dx\n"
        "int $0x10\n"
        :
        :
        : "ah", "al", "bh", "cx", "dx"
    );
    //return to columb 0
    pchar('\r');
    //return to row 0
    asm volatile (
        "movb $0x02, %%ah\n"
        "movb $0x00, %%bh\n"
        "movb $0x00, %%dh\n"
        "movb $0x00, %%dl\n"
        "int $0x10\n"
        :
        :
        : "ah", "bh", "dh", "dl"
    );
}

//moves the cursor up by one
void mvcursor_up(void){
    asm(
        "movb $0x03, %%ah\n"//        ; Get cursor position
        "movb $0x0, %%bh\n"//          ; Page number
        "int $0x10\n"
        "inc %%dh\n"//             ; Move up
        "movb $0x02, %%ah\n"//        ; Set cursor position
        "int $0x10\n"
        :
        :
        : "ah", "bh", "dl", "ah"
    );
}

//move the cursor down by one
void mvcursor_down(void){
    asm(
        "movb $0x03, %%ah\n"//        ; Get cursor position
        "movb $0x0, %%bh\n"//          ; Page number
        "int $0x10\n"
        "dec %%dh\n"//             ; Move down
        "movb $0x02, %%ah\n"//        ; Set cursor position
        "int $0x10\n"
        :
        :
        : "ah", "bh", "dl", "ah"
    );
}

//moves the cursor to the right
void mvcursor_right(void){
    asm(
        "movb $0x03, %%ah\n"//        ; Get cursor position
        "movb $0x0, %%bh\n"//          ; Page number
        "int $0x10\n"
        "inc %%dl\n"//             ; Move right
        "movb $0x02, %%ah\n"//        ; Set cursor position
        "int $0x10\n"
        :
        :
        : "ah", "bh", "dl", "ah"
    );
}

//moves the cursor to the left
void mvcursor_left(void){
    asm(
        "movb $0x03, %%ah\n"   //      ; Get cursor position
        "movb $0x0, %%bh\n"    //      ; Page number
        "int $0x10\n"
        "dec %%dl\n"          //       ; Move left
        "movb $0x02, %%ah\n"   //      ; Set cursor position
        "int $0x10\n"
        :
        :
        : "ah", "bh", "dl", "ah"
    );
}

//move the cursor relative to the current position
void mvcursor(short difx, short dify){
    if(difx < 0){
        for(short i = 0; i > difx; i--){
            mvcursor_left();
        }
    }else{
        for(short i = 0; i < difx; i++){
            mvcursor_right();
        }
    }
    if(dify < 0){
        for(short i = 0; i > dify; i--){
            mvcursor_up();
        }
    }else{
        for(short i = 0; i < dify; i++){
            mvcursor_down();
        }
    }
}

//====================================================

//===================Integer Funcs====================

//turns inputed bcds into outputed integers
byte bcdtoi(byte bcd){
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}


//returns the digit count of the inputed number
byte dc(int num){
    byte result = 1;
    if (num >= 10) {
        result = 1 + dc(num / 10);
    }
    return result;
}


//puts the string form of num into output
void uitos(int num, char* output){
    byte digits = dc(num);
    char str[digits + 1]; // +1 for null terminator
    int curnum;
    
    for (int i = 0; i < digits; i++) {
        curnum = num;
        curnum %= intpow(10, digits - i);               // Remove digits to the left
        curnum -= (curnum % intpow(10, digits - i - 1)); // Remove digits to the right
        curnum /= intpow(10, digits - i - 1);           // Isolate current digit
        str[i] = curnum + '0';
    }

    str[digits] = '\0';           // Null-terminate the string
    strcpy(str, output);          // Copy into output
}

//---------------------Math Funcs---------------------

//evaluates base^exp
int intpow(int base, short exp){
    int answer = 1;
    for(short i = 0; i < exp; i++){
        answer *= base;
    }
    return answer;
}

//====================================================