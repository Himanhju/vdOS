/*
\n (0x0A): move cursor down one line

\r (0x0D): return to column 0 of current line

\b (0x08): backspace

\f (0x0C): form feed (clears screen on some terminals)

\a (0x07): bell
*/
//======================defines======================

#define VGA_TERM_WIDTH 160
#define VGA_TERM_HEIGHT 25

#define false 0
#define true 1

#define NULL ((void *)0)

#define MAX_ARG_SIZE 64

#define MAX_FILES 8
#define MAX_FILENAME_SIZE 32

#define va_start(v, l)	__builtin_va_start(v, l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v, T)	__builtin_va_arg(v, T)
#define va_copy(d, s)	__builtin_va_copy(d, s)

//=====================Type Defs=====================

typedef unsigned char      bool; // true or false variable {1 byte}

typedef unsigned char      byte; // a single byte for memory efficiency {1 byte}

typedef unsigned char      uint8_t; // 8 bit positive integer {1 byte}
typedef signed char        int8_t; // integer spreading 4 bits into negative and positive {1 byte}

typedef unsigned short     uint16_t; // 16 bit positive integer {2 bytes}
typedef signed short       int16_t; // integer spreading 8 bits into negative and positive {2 bytes}

typedef unsigned int       uint32_t; // 32 bit positive integer {4 bytes}
typedef signed int         int32_t; // integer spreading 16 bits into negative and positive {4 bytes}

typedef unsigned long long  uint64_t; // 64 bit positive integer {8 bytes}
typedef signed long long    int64_t; // integer spreading 32 bits into negative and posative {8 bytes}

typedef __builtin_va_list va_list;

//a struct for keypresses
typedef struct key{
    char ascii;
    char scan;
}key;

// a struct for a time variable
typedef struct time{
    byte hour; // the hour
    byte minute; // the minute
    byte second; // the second
}time;

//the struct for a file
typedef struct file{
    char filename[MAX_FILENAME_SIZE]; // first 8 characters is the filename (9th is null term)
    uint32_t sector_start; // the starts of the sectors used in storage (42-...)
    int8_t sectors_used; // amount of sectors used by the file (1-127?)
    char *contents; // pointer to where to put the contents after it was read
}file;



//====================Prototypes=====================

//===== Shell =====

void parse(char* input);
void halt(void);
void get(char* arg);
void print_help(void);
void poweroff(void);
void touch(char* arg1, char* arg2);
void ls(char *args1);
void cat(char *arg1);
void edit(char *arg1);

//===== FS =====

void init_FS(void);
void new_file(byte file_index, const char* filename, uint32_t sector_start, byte sectors_used, void *contents);

//----- File IO -----

void write(file fil);
void read(file fil);

//_____ Sector IO _____

byte write_sect(uint32_t sect);
byte read_sect(uint32_t sect);
void clrsect_buf(void);

//===== Time =====

unsigned long long get_uptime(void);

//===== String Sunctions =====

short strlen(const char* str);
byte strncmp(const char* str1, const char* str2, const short n);
byte strcmp(const char* str1, const char* str2);
void strcpy(const char* src, char* dest, const short offset);
void strncpy(const char* src, char* dest, const short n, const short offset);
unsigned int stoui(const char* str);

//===== IO =====

void get_key_info(void);
char get_key(void);
char get_scan(void);
void pchar(char c);
void pcharAP(char c, byte x, byte y);
byte get_str(const uint16_t max, char* sto);
void pstr(const char* str);
void print(char* str);
void pint(const int num);
void newline(void);

//===== Cursor =====

void clrscrn(void);
void mvcursor_up(void);
void mvcursor_down(void);
void mvcursor_right(void);
void mvcursor_left(void);
void mvcursor(const byte difx, const byte dify);
void set_curpos(const byte x, const byte y);

//===== Int Functions =====

byte bcdtoi(const byte bcd);
void uitos(const int num, char* output);

//----- Math Functions -----

int intpow(const int base, const short exp);

//=================global varialbles=================

//===== FS Vars =====
file files[MAX_FILES];

//===== Multi-Purpose Vars =====
unsigned short MPInt = 0;
time tim;
char MPBuffer[512];

key MPKey;

//===== Memory Vars =====

//----- Video Regions ----- 
volatile char *vram_TM = (volatile char *)0x000B8000; // vram in character mode
volatile char *vram_GM = (volatile char *)0x000A0000; // vram in pixel/graphics mode

//----- FS Regions -----
char *sect_buf = (char *)0x00005000; // the sector buffer for the syscalls

//===================================================

//no more bytes can be before this as they would be ran first (only compiler commands[#defines, #pragmas, etc.] and prototypes)
void start(){
    clrscrn();
    
    init_FS();

    char in[512];

    strcpy("BuiltIn.bkl", files[0].filename, 0);
    files[0].sector_start = 0;
    files[0].sectors_used = 50;

    while(1){
        pstr(">>> ");
        for(unsigned short i = 0; i < 512; i++){
            in[i] = '\0';
            MPBuffer[i] = '\0';
            sect_buf[i] = '\0';
        }
        MPInt = 0;

        get_str(512, in);


        parse(in);
    }
}
//=======================Shell=======================

//parses the input and puts it into args
void parse(char* input){
    int input_len = strlen(input);
    int num_input = 0;
    for(int c = 0; c < input_len; c++) {
        if(input[c] == ' '){
            num_input++;
        }
    }
    num_input++;
    int input_lens[num_input];
    int input_starts[num_input];
    int cur_input = 0;
    for(int o = 0; o < num_input; o++){
        input_lens[o] = 0;
        input_starts[o] = 0;
    }
    for(int c = 0; c < input_len; c++){
        if(input[c] == ' '){ cur_input++; input_starts[cur_input] = c + 1; continue;}                                             
        input_lens[cur_input]++;
    }
    char args[num_input][MAX_ARG_SIZE];

    for(int i = 0; i < num_input; i++){
        for(int o = 0; o < 64; o++) {
            args[i][o] = '\0';
        }
        for(int c = input_starts[i]; c < input_starts[i] + input_lens[i]; c++) {
            args[i][c - input_starts[i]] = input[c];
        }
    }
    /*
    for(int i = 0; i < num_input; i++){
        pint(i);
        pstr(": ");                         //debuger for parsing
        pstr(args[i]);
        newline();
    }
    */

    //args[0] == the command
    if(strncmp(args[0], "halt", 5) == 0){
        halt();
    }else if(strncmp(args[0], "get", 4) == 0){
        get(args[1]);
    }else if(strncmp(args[0], "clear", 6) == 0){
        clrscrn();
        return;
    }else if(strncmp(args[0], "echo", 5) == 0){
        for(byte i = 1; i < num_input + 1; i++){
            if(args[i][0] == '\0'){
                pchar(' ');
                continue;
            }
            pstr(args[i]);
            pchar(' ');
        }
    }else if(strncmp(args[0], "Ereset", 7) == 0){
        start(); // Emergency Reset in case of corruption (emergency because its very bad and inefficient)
    }else if(strncmp(args[0], "help", 5) == 0){
        print_help();
    }else if(strncmp(args[0], "poweroff", 9) == 0){
        poweroff();
    }else if(strncmp(args[0], "touch", 6) == 0){
        touch(args[1], args[2]);
    }else if(strncmp(args[0], "ls", 3) == 0){
        ls(args[1]);
    }else if(strncmp(args[0], "cat", 4) == 0){
        cat(args[1]);
    }else if(strncmp(args[0], "edit", 5) == 0){
        edit(args[1]);
    }else{
        pstr(args[0]);
        pstr(": command not found");
    }
    newline();
}

//halts the cpu and prevents further execution
void halt(void){
    asm(
        "cli\n"
        "hlt\n"
    );
}

//gets variables and user things
void get(char* arg){
    if(strncmp(arg, "scan", 5) == 0){
        pstr("scan code: ");
        pint((int)get_scan());
    }else if(strncmp(arg, "ascii", 6) == 0){
        pstr("Ascii key: ");
        uitos(get_key(), MPBuffer);
        pstr(MPBuffer);
    }else if(strncmp(arg, "uptime", 7) == 0){
        pint(get_uptime());
    }else if(strncmp(arg, "time", 5) == 0){
        MPInt = get_uptime();
        tim.second = MPInt % 60;
        tim.minute = (MPInt - tim.second) / 60;
        tim.hour   = (tim.minute - (tim.minute % 60)) / 60;
        tim.minute = tim.minute - (tim.hour * 60);
        pint(tim.hour);
        pchar(':');
        pint(tim.minute);
        pchar(':');
        pint(tim.second);
    }else{
        pstr("Usage: get SUBCOMMAND");
        newline();
        newline();
        pstr("ascii         Gets the ascii code of the key next pressed.");
        newline();
        pstr("scan          Gets the scan code of the next key pressed.");
        newline();
        pstr("uptime        Gets the \"uptime\" of the OS based on the BIOS ticks.");
        newline();
        pstr("time          Gets the time from the uptime");
        newline();
    }
}

//prints the help menu
void print_help(void){
    pstr("halt          Halts the OS and prevents further execution.");
    newline();
    pstr("get           Gets things like the time and scan codes.");
    newline();
    pstr("clear         Clears the screen from all text.");
    newline();
    pstr("echo          Prints all further arguments to the screen.");
    newline();
    pstr("Ereset        Emergency resets the OS incase of problems.");
    newline();
    pstr("poweroff      Powers off the computer and stops the OS.");
    newline();
    pstr("touch         Creates Files of a given size.");
    newline();
    pstr("cat           Prints the contents of a file.");
    newline();
    pstr("edit          Opens a text editor to edit a file (vdTE)");
    newline();
    pstr("ls            List active files");
    newline();
    pstr("help          Prints this help text.");
    newline();
    newline();
    pstr("Note: You can add --help to supported commands to see their help menus");
}

//syscall for powering off the computer and saves files to sector 41 (fs index sector)
void poweroff(void){
    asm volatile (
        "movb $0x00, %%dh\n"
        "int $0x27\n" //syscall
        :
        :
        : "dh"
    );
    pstr("Could not Poweroff");
}

//command to create new files
void touch(char* arg1, char* arg2){
    if(strncmp(arg1, "--help", 7) == 0){
        pstr("Touch: Makes new files");
        newline();
        newline();
        pstr("Usage: touch filename");
        newline();
        pstr("Usage: touch filename sectors");
    }else if(arg1[0] != '\0'){
        // if args[1] is the filename
        if(arg2[0] != '\0'){
            // if args[2] is the amount of sectors (Usage: 2)
            byte index = 1;

            while(index < MAX_FILES){
                if(strncmp(arg1, files[index].filename, strlen(arg1)) == 0){
                    pstr("ERROR: Cannot make new file: file already exists");
                    return;
                }
                if(files[index].sector_start > 0){
                    index++;
                    continue; // if its used;
                }
                MPInt = (files[index - 1].sector_start + files[index - 1].sectors_used) - 1;
                new_file(index, arg1, MPInt, stoui(arg2), NULL);
                break;
            }
            
        }else{
            // if args[2] is not the amount of sectors (Usage: 1)
            byte index = 1;

            while(index < MAX_FILES){
                if(strncmp(arg1, files[index].filename, strlen(arg1)) == 0){
                    pstr("ERROR: Cannot make new file: file already exists");
                    newline();
                    return;
                }
                //if(files[index].sector_start > 0){
                //    index++;
                //    continue; // if its used;
                //}
                MPInt = (files[index - 1].sector_start + files[index - 1].sectors_used) + 1;
                new_file(index, arg1, MPInt, 1, NULL);
                break;
            }
        }
        mvcursor_up();
        return;
    }else{
        pstr("touch: missing file operand");
        newline();
        pstr("Try \'touch --help\' for more information.");
    }
}

//list the files in a directory (defaults to . {current directory})
void ls(char *args1){
    strcpy(args1, MPBuffer, 0); // put to not givve me a warning/error for not using arg1
    for(byte i = 0; i < MAX_FILES; i++){
        pstr(files[i].filename);
        pchar(' ');
    }
}

//print the contents of a file
void cat(char *arg1){
    if(strncmp(arg1, "--help", 7) == 0){
        pstr("Usage: cat [filename]");
        newline();
        pstr("prints the contents of a file to the terminal");
        newline();
        newline();
        pstr("Note: If no file is given then it assumes the users input");
    }else if(arg1[0] != '\0'){
        MPInt = 0;
        for(byte i = 0; i < MAX_FILES; i++){
            if(strncmp(arg1, files[i].filename, strlen(arg1)) == 0){
                // if the file is found
                char contents[files[i].sectors_used * 0x200]; // make an array for the contents with the size of the file
                files[i].contents = contents;
                MPInt = files[i].sectors_used * 0x200;
                read(files[i]);
                pstr(contents);
                files[i].contents = NULL;
                MPInt = true;
                break;
            }
        }
        if(MPInt != true){
            // if no file of that name was found
            pstr("cat: ");
            pstr(arg1);
            pstr(": No such file");
        }
    }else{
        while(1){
            tim.second = get_str(512, MPBuffer);
            if(tim.second == 1){
                break;
            }
            pstr(MPBuffer);
            newline();
        }
    }
}

//a text editor for ... editing files
void edit(char *arg1) {
    clrscrn();
    for (byte i = 0; i < MAX_FILES; i++) {
        if (strncmp(files[i].filename, arg1, strlen(files[i].filename)) == 0) {
            // Read the file's first sector into sect_buf
            read_sect(files[i].sector_start);
            print(sect_buf); // Print the contents of the sector buffer

            bool editing = true;
            short content_index = strlen(sect_buf); // Start at the end of the current content

            while (editing) {
                get_key_info();

                if (MPKey.scan == 1) { // Escape key
                    set_curpos(0, 25);
                    get_str(512, MPBuffer);
                    if (strcmp(":w", MPBuffer) == 0) {
                        // Write the updated contents of sect_buf back to the file's first sector
                        write_sect(files[i].sector_start);

                        // Clear the command line (row 25)
                        set_curpos(0, 25);
                        for (int j = 0; j < VGA_TERM_WIDTH; j++) {
                            pchar(' '); // Overwrite the line with spaces
                        }

                        // Reset cursor to (0, 0)
                        set_curpos(0, 0);
                        pstr("File saved.\n");
                    } else if (strcmp(":q", MPBuffer) == 0) {
                        clrscrn();
                        editing = false;
                    } else {
                        pstr("Invalid command. Use :w to save or :q to quit.");
                    }
                    set_curpos(0, 0);
                } else if (MPKey.ascii == 8) { // Backspace
                    if (content_index > 0) {
                        content_index--;
                        sect_buf[content_index] = '\0'; // Clear the last character
                        mvcursor_left();
                        pchar(' '); // Overwrite the character on the screen
                        mvcursor_left();
                    }
                } else if (MPKey.ascii == '\r') { // Enter
                    sect_buf[content_index++] = '\n';
                    newline();
                } else if (MPKey.ascii != 0) { // Regular character
                    sect_buf[content_index++] = MPKey.ascii;
                    pchar(MPKey.ascii);
                }
            }
            clrscrn();
            return;
        }
    }

    pstr("edit: File not found: ");
    pstr(arg1);
    return;
}

//===================================================

//=========================FS========================

//initializes the filesystem
void init_FS(void){
    for(byte i = 0; i < MAX_FILES; i++){ // run on every single file
        files[i].contents = NULL; // set the contents pointer for the current file to nothing*
        for(byte o = 0; o < MAX_FILENAME_SIZE; o++){// run on every character of the current filename
            files[i].filename[o] = '\0';// set the current character to \0 (null terminator)
        }
        files[i].sectors_used =  -1; // set the current sectors used to the lowest possible value
    }
}

//creates a new file
void new_file(byte file_index, const char* filename, uint32_t sector_start, byte sectors_used, void *contents){
    strncpy(filename, files[file_index].filename, strlen(filename), 0);// set the filename
    files[file_index].sector_start = sector_start; // set the starting sector
    files[file_index].sectors_used = sectors_used; // set the amount of sectors used
    files[file_index].contents = contents; // set the contents pointer
}

//----------------------file io-----------------------

//writes data to a file (assumes the inputed file's contents points to the data to write)
void write(file fil){// posible optimization: change sector offset to byte offset/just offset
    volatile byte sectors = MPInt / 512; // the amount of sectors to read
    if(MPInt % 512 > 0){ // if bytes is greater then a multiple of 512
        sectors += 1;// then add one for another sector and the extra data
    }
    for(byte i = 0; i < sectors; i++){ // run for every sector used
        if(i > fil.sectors_used){ //  minus 1 because the sectors used part includes the start
            break; // if we've exited the/reached the end of the file, then stop reading
        }
        for(unsigned short s = 0; s < 512; s++){
            sect_buf[s] = fil.contents[s + (i * 512)];// copy current 512 bytes of the sector buffer into the contents at its position
        }
        write_sect(fil.sector_start + i); // write the sector buffor to the current sector
        clrsect_buf(); // "flushes" (clears) the contents of the sector buffer
    }
}

//reads data from a file and puts it into the contents pointer
void read(file fil){ // posible optimization: change sector offset to byte offset/just offset
    byte sectors = MPInt / 512; // the amount of sectors to read
    if(MPInt % 512 > 0){ // if bytes is greater then a multiple of 512
        sectors += 1;// then add one for another sector and the extra data
    }
    for(byte i = 0; i < sectors; i++){ // run for every sector used
        if(i > fil.sectors_used - 1){ //  minux 1 because the sectors used part includes the start
            break; // if we've exited the/reached the end of the file, then stop reading
        }
        read_sect(fil.sector_start + i); // set the sector buffer to the current sector
        for(unsigned short s = 0; s < 512; s++){
            fil.contents[s + (i * 512)] = sect_buf[s];// copy current 512 bytes of the sector buffer into the contents at its position
        }
        clrsect_buf(); // "flushes" (clears) the contents of the sector buffer
    }
}

//writes the sector buffer to a sector
byte write_sect(uint32_t sect) {
    byte status;

    asm volatile (
        "movb $0x02, %%dh\n"
        "int $0x27\n"
        
        : "=a"(status)                        // AL output
        : "c"(sect)                           // CL input
        : "dx"
    );

    return status;
}

//puts the contents of a sector into the sector buffer
byte read_sect(uint32_t sect) {
    byte status;

    asm volatile (
        "movb $0x01, %%dh\n"
        "int $0x27\n"
        
        : "=a"(status)                        // AL output
        : "c"(sect)                           // CL input
        : "dh"
    );

    return status;
}

//clears the sector buffer
void clrsect_buf(void){
    for(short i = 0; i < 512; i++){
        sect_buf[i] = '\0';
    }
}

//===================================================

//========================Time=======================

//gets uptime from boot
unsigned long long get_uptime(void) {
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

    ticks = ((unsigned short)high << 16) | low;

    return ticks / 18;  // rough approximation
}

//====================================================

//====================String Funcs===================

//returns the lengths of the given string
short strlen(const char* str){
    short len;
    for(len = 0; str[len] != '\0';len++);
    return len;
}

//compares the first n bytes, returns 0 if they are the same
byte strncmp(const char* str1, const char* str2, const short n){
    for(short i = 0; i < n; i++){
        if(str1[i] != str2[i]){
            return 2;
        }
    }
    return 0;
}

//returns 0 if they are the same
byte strcmp(const char* str1, const char* str2){
    return strncmp(str1, str2, strlen(str1));
}

//copies the first string into the second string
void strcpy(const char* src, char* dest, const short offset){
    for(int i = offset; i < strlen(src); i++){       //if necisarry then also add return the amount of bytes copied
        dest[i - offset] = src[i];
    }
}

//copies the first n bytes from the first sting into the second string
void strncpy(const char* src, char* dest, const short n, const short offset){
    for(int i = offset; i < offset + n; i++){       //if necisarry then also add return the amount of bytes copied
        dest[i] = src[i];
    }
}

//converts a string into an unsigned integer
unsigned int stoui(const char* str){
    unsigned int ret = 0;
    MPInt = strlen(str);

    for(short i = 0; i < MPInt; i++){
        char c = str[i];
        if(c >= '0' && c <= '9'){
            ret = ret * 10 + (c - '0');
        } else {
            break; // or handle error
        }
    }
    return ret;
}

//====================================================

//=========================IO=========================

//get the info of the current keystroke
void get_key_info(void){
    asm volatile (
        "movb $0x00, %%ah\n"
        "int $0x16\n"
        "movb %%al, %0\n"
        "movb %%ah, %1\n"
        : "=m"(MPKey.ascii), "=m"(MPKey.scan)        // output: %0 gets value from AL
        :                  // no input
        : "ah", "al"       // clobbered: we're modifying AH and AL
    );
}

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
byte get_str(const uint16_t max, char* sto){
    char cha;
    //char scan;
    for(unsigned short cur = 0; cur < max;cur++){
        cha = get_key();
        //scan = get_scan();
        if(cha == '\r'){
            for(uint16_t o = cur; o < max; o++){
                sto[o] = '\0'; // over write further data with zeroes to prevent bleed
            }
            break;
        }else if(cha == 8){
			if(cur < 1){
				cur--;
				continue;
			}
			cur -= 2;
			pchar(cha);
            pchar(0);
            pchar(cha);
			continue;
		}else if(cha == 3){
            pstr("^C");
            return 1;
        }
        sto[cur] = cha;
        pchar(cha);
    }
    newline();
    return 0;
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

//prints the character held by c at the position (x, y)
void pcharAP(char c, byte x, byte y){
    vram_TM[(x + (y * VGA_TERM_WIDTH)) - 1] = c;
}

//prints and entire string (not advised to use a newline character in this)
void pstr(const char* str){
    while (*str) {
        pchar(*str++); //fix: cannot print variables, only constants
    }
}

//actual multi purpose print
void print(char* str){
    for(int i = 0 ; i < strlen(str); i++){
        if(str[i] == '\n'){
            newline();
            continue;
        }
        pchar(str[i]);
    }
}


//prints an integer
void pint(const int num){
    char string[255];
    uitos(num, string);
    pstr(string);
}

//prints a newline
void newline(void){
    pstr("\n\r"); // \n moves it down and \r returns it to the left
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
        "dec %%dh\n"//             ; Move up
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
        "inc %%dh\n"//             ; Move down
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
void mvcursor(const byte difx, const byte dify){
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

//set the cursors position
void set_curpos(const byte x, const byte y){
    asm(
        "movb $0x03, %%ah\n"//        ; Get cursor position
        "movb $0x0, %%bh\n"//          ; Page number
        "int $0x10\n"
        "mov %0, %%dl\n"
        "mov %1, %%dh\n"
        "movb $0x02, %%ah\n"//        ; Set cursor position
        "int $0x10\n"
        :
        : "r"(x), "r"(y)
        : "ah", "bh", "dl", "ah"
    );
}

//====================================================

//===================Integer Funcs====================

//turns inputed bcds into outputed integers
byte bcdtoi(const byte bcd){
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}


//returns the digit count of the inputed number
byte dc(const int num){
    byte result = 1;
    if (num >= 10) {
        result = 1 + dc(num / 10);
    }
    return result;
}


//puts the string form of num into output
void uitos(const int num, char* output){
    byte digits = dc(num);
    int curnum;
    
    for (int i = 0; i < digits; i++) {
        curnum = num;
        curnum %= intpow(10, digits - i);               // Remove digits to the left
        curnum -= (curnum % intpow(10, digits - i - 1)); // Remove digits to the right
        curnum /= intpow(10, digits - i - 1);           // Isolate current digit
        output[i] = curnum + '0';
    }

    output[digits] = '\0';           // Null-terminate the string
}

//---------------------Math Funcs---------------------

//evaluates base^exp
int intpow(const int base, const short exp){
    int answer = 1;
    for(short i = 0; i < exp; i++){
        answer *= base;
    }
    return answer;
}

//====================================================
