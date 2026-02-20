
// CoreSys TC Compiler - STRICT TR API ONLY VERSION
// Only supports documented tc.* functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 65536
#define MAX_VARS 1024
#define MAX_TOKENS 128
#define MAX_TOKEN_LEN 64

static int compile_line(char *line);

int return_value = 0;
static int in_license = 0;

// Global init flags
static int has_init = 0;
static int has_retinit = 0;
int stack_buffer_size = 256; // default
// static int has_loopsinit = 0;
// static int has_graphicsinit = 0;
// static int has_langinit = 0;
// static int has_guiinit = 0;
// static int has_functioninit = 0;

typedef struct {
    char name[64];
    int offset;
} Var;

static Var vars[MAX_VARS];
static int var_count = 0;

static FILE *out;


// Call this when tc.init() is executed
static void set_init_called()
{
    has_init = 1;
}

// Call this when tc.ret.init() is executed
static void set_retinit_called()
{
    has_retinit = 1;
}

// Check if tc.init() has been called
static int is_init_called()
{
    return has_init;
}

// Check if tc.ret.init() has been called
static int is_retinit_called()
{
    return has_retinit;
}

static int get_var_offset(const char *name)
{
    for(int i=0;i<var_count;i++)
        if(strcmp(vars[i].name,name)==0)
            return vars[i].offset;

    if(var_count>=MAX_VARS){
        printf("too many vars\n");
        exit(1);
    }

    strcpy(vars[var_count].name,name);
    vars[var_count].offset = -8*(var_count+1);

    return vars[var_count++].offset;
}

static void normalize(char *s)
{
    while(isspace(*s)) memmove(s,s+1,strlen(s));
    int n=strlen(s);
    while(n>0 && (isspace(s[n-1])||s[n-1]==';')) s[--n]=0;
}

static int is_number(const char *s,long *v)
{
    char *e;
    long val=strtol(s,&e,0);
    if(*e==0){
        *v=val;
        return 1;
    }
    return 0;
}

static void load(const char *tok,const char *reg)
{
    long v;
    if(is_number(tok,&v))
        fprintf(out," mov %s,%ld\n",reg,v);
    else
        fprintf(out," mov %s,[rbp%d]\n",reg,get_var_offset(tok));
}

static void store(const char *tok,const char *reg)
{
    fprintf(out," mov [rbp%d],%s\n",get_var_offset(tok),reg);
}

static void emit_header(const char *entry)
{
    fprintf(out,
        "global _start\n"
        "section .text\n"
        "tc_newline:\n  db 10\n"
        "_start:\n"
        " call %s\n"
        " mov rdi,rax\n"
        " mov rax,60\n"
        " syscall\n\n",
        entry);
}

static void emit_runtime()
{
    fprintf(out,

"tc_init:\n ret\n"
"tc_func_init:\n ret\n"
"tc_loops_init:\n ret\n"
"tc_graphics_init:\n ret\n"
"tc_lang_init:\n ret\n"
"tc_lang_run:\n ret\n"

"tc_ret_init:\n"
"   mov rsp, rbp\n"
"   pop rbp\n"
"   mov rax, 0\n"

"tc_print:\n"
"    push rbp\n"
"    mov rbp, rsp\n"
"    sub rsp, 1             ; allocate 1 byte on stack\n"
"    mov [rsp], dil         ; store character\n"
"\n"
"    mov rax, 1             ; sys_write\n"
"    mov rdi, 1             ; stdout\n"
"    lea rsi, [rsp]         ; pointer to character\n"
"    mov rdx, 1             ; write 1 byte\n"
"    syscall\n"
"\n"
"    add rsp, 1             ; free stack\n"
"    mov rsp, rbp\n"
"    pop rbp\n"
"    ret\n"
"\n"
"tc_printf:\n"
"    push rbp\n"
"    mov rbp,rsp\n"
"    call tc_print\n"
"    mov rsp,rbp\n"
"    pop rbp\n"
"    ret\n"
"; --- Print null-terminated string pointed to by rdi ---\n"
"tc_write_string:\n"
"    push rbp\n"
"    mov rbp,rsp\n"
"    mov rsi,rdi          ; rsi = pointer to string\n"
".loop:\n"
"    mov al,[rsi]         ; load current byte\n"
"    test al,al\n"
"    je .done\n"
"    mov rax,1            ; sys_write\n"
"    mov rdi,1            ; stdout\n"
"    mov rdx,1            ; write 1 byte\n"
"    syscall\n"
"    inc rsi              ; move to next character\n"
"    jmp .loop\n"
".done:\n"
"    pop rbp\n"
"    ret\n"
);
}

// Returns 1 if the line is a comment (line comment, block comment, or license comment), 0 otherwise
static int is_comment(const char *line)
{
    // Trim leading whitespace
    while (*line && isspace(*line)) line++;

    // Om vi redan är inne i ett LICENSE-block, kolla om det slutar
    if (in_license) {
        if (strstr(line, "~^")) {
            in_license = 0; // LICENSE-block slut
        }
        return 1; // Hoppa över alla rader inom LICENSE
    }

    // LICENSE-block start (^~)
    if (strncmp(line, "^~", 2) == 0) {
        in_license = 1;
        return 1;
    }

    // Line comment starting with #
    if (*line == '#')
        return 1;

    // Line comment starting with //
    if (strncmp(line, "//", 2) == 0)
        return 1;

    // Block comment ending (\\) at line end
    size_t len = strlen(line);
    if (len >= 2 && line[len - 2] == '\\' && line[len - 1] == '\n')
        return 1;

    return 0; // Inte en kommentar
}
int tokenize_line(char *line, char tokens[MAX_TOKENS][MAX_TOKEN_LEN]) {
    int count = 0;
    int len = strlen(line);
    int pos = 0;

    while (pos < len) {
        // Hoppa över mellanslag
        while (isspace(line[pos])) pos++;

        if (pos >= len) break;

        // Hoppa över kommentarer
        if (line[pos] == '#' || (line[pos] == '/' && line[pos+1] == '/')) break;

        // Hantera symboler som egna tokens
        if (strchr("(){};,", line[pos])) {
            tokens[count][0] = line[pos];
            tokens[count][1] = '\0';
            count++;
            pos++;
            continue;
        }

        // Samla bokstäver/numeriska tecken
        int tlen = 0;
        while (pos < len && !isspace(line[pos]) && !strchr("(){};,#", line[pos])) {
            tokens[count][tlen++] = line[pos++];
            if (tlen >= MAX_TOKEN_LEN-1) break;
        }
        tokens[count][tlen] = '\0';
        count++;
    }
    return count;
}

static int compile_line(char *line) {
    normalize(line);

    // Trim trailing whitespace och semikolon
    int len = strlen(line);
    while(len>0 && (isspace(line[len-1])||line[len-1]==';')) line[--len]='\0';

    // Ta bort kommentarer
    char *comment = strchr(line,'#');
    if(comment) *comment='\0';

    // Hoppa tomma/kommentarer
    while(*line && isspace(*line)) line++;
    if(*line=='\0') return 0;
    if(is_comment(line)) return 0;

    char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
    int num_tokens = tokenize_line(line,tokens);

    // ================== Token-baserad parsing ==================
    if(num_tokens >= 5 &&
        strcmp(tokens[0], "func") == 0 &&
        strcmp(tokens[2], "(") == 0 &&
        strcmp(tokens[3], ")") == 0 &&
        strcmp(tokens[4], "{") == 0)
    {
        fprintf(out,
                    "%s:\n"
                    " push rbp\n"
                    " mov rbp,rsp\n"
                    " sub rsp,%d\n",
                    tokens[1],
                    stack_buffer_size);

    }
    else if(num_tokens == 2 && strcmp(tokens[0], "ret") == 0 &&
            strcmp(tokens[1], "0") == 0)
    {
        fprintf(out,"    mov rax,0\n    ret\n");
    }
    else if(num_tokens >= 3 && strcmp(tokens[0], "tc.init") == 0 &&
            strcmp(tokens[1], "(") == 0 && strcmp(tokens[2], ")") == 0)
    {
        fprintf(out,"    call tc_init\n");
        set_init_called();
    }
    else if(num_tokens >= 3 && strcmp(tokens[0], "tc.ret.init") == 0 &&
            strcmp(tokens[1], "(") == 0 && strcmp(tokens[2], ")") == 0)
    {
        fprintf(out,"    call tc_ret_init\n");
        set_retinit_called();
    }
    else if(num_tokens >= 3 && strcmp(tokens[0], "tc.loops.init") == 0 &&
            strcmp(tokens[1], "(") == 0 && strcmp(tokens[2], ")") == 0)
    {
        fprintf(out,"    call tc_loops_init\n");
    }
    else if(num_tokens >= 3 && strcmp(tokens[0], "tc.graphics.init") == 0 &&
            strcmp(tokens[1], "(") == 0 && strcmp(tokens[2], ")") == 0)
    {
        fprintf(out,"    call tc_graphics_init\n");
    }
    else if(num_tokens >= 3 && strcmp(tokens[0], "tc.lang.init") == 0 &&
            strcmp(tokens[1], "(") == 0 && strcmp(tokens[2], ")") == 0)
    {
        fprintf(out,"    call tc_lang_init\n");
    }
    else if(num_tokens >= 4 && strcmp(tokens[0], "tc.lang.run") == 0 &&
            strcmp(tokens[1], "(") == 0 && strcmp(tokens[num_tokens-1], ")") == 0)
    {
        char arg[64];
        strncpy(arg, tokens[2], 63);
        arg[63] = '\0';
        normalize(arg);
        load(arg,"rdi");
        fprintf(out,"    call tc_lang_run\n");
    }
    else if(num_tokens >= 4 && strcmp(tokens[0], "tc.print") == 0 &&
        strcmp(tokens[1], "(") == 0 && strcmp(tokens[num_tokens-1], ")") == 0)
    {
        char arg[256];
        strcpy(arg, tokens[2]);
        for(int i=3;i<num_tokens-1;i++){
            strcat(arg," ");
            strcat(arg,tokens[i]);
        }
        normalize(arg);

        if(arg[0]=='$'){  // variable
            load(arg+1,"rdi");
            fprintf(out,"    call tc_print\n");
        }
        else {
            long val;
            if(is_number(arg,&val)){
                fprintf(out,"    mov rdi,%ld\n    call tc_print\n",val);
            } else {
                static int str_count = 0;
                char label[64];
                sprintf(label,"lit_%d",str_count++);
                fprintf(out,"section .data\n%s db \"%s\",0\nsection .text\n",label,arg);
                fprintf(out,"    lea rdi,[rel %s]\n    call tc_write_string\n",label);
            }
        }
    }
    else if(num_tokens >= 4 && strcmp(tokens[0], "tc.printf") == 0 &&
        strcmp(tokens[1], "(") == 0 && strcmp(tokens[num_tokens-1], ")") == 0)
{
    char arg[256];
    strcpy(arg, tokens[2]);
    for(int i=3;i<num_tokens-1;i++){
        strcat(arg," ");
        strcat(arg,tokens[i]);
    }
    normalize(arg);

    if(arg[0]=='$'){  // variable
        load(arg+1,"rdi");
        fprintf(out,"    call tc_print\n");   // leave as-is for variables
    }
    else {
        long val;
        if(is_number(arg,&val)){
            fprintf(out,"    mov rdi,%ld\n    call tc_printf\n",val);
        } else {
            static int str_count = 0;
            char label[64];
            sprintf(label,"lit_%d",str_count++);
            fprintf(out,"section .data\n%s db \"%s\",0\nsection .text\n",label,arg);

            // NEW: emit string print + newline
            fprintf(out,
                "    lea rdi,[rel %s]\n"
                "    call tc_write_string\n"
                "    mov rax,1\n"
                "    mov rdi,1\n"
                "    mov rsi,tc_newline\n"
                "    mov rdx,1\n"
                "    syscall\n",
                label);
        }
    }
}
    else if(num_tokens == 1 && strcmp(tokens[0], "}") == 0)
    {
        // stäng funktion/block
    }
    // ================== Matematik ==================
    else if(num_tokens >= 6 &&
       strcmp(tokens[0],"tc.math.assign")==0 &&
       strcmp(tokens[1],"(")==0 &&
       strcmp(tokens[3],",")==0 &&
       strcmp(tokens[5],")")==0)
    {
        char a[64],b[64];
        strcpy(a,tokens[2]);
        strcpy(b,tokens[4]);
        normalize(a); normalize(b);
        load(b,"rax");
        store(a,"rax");
    }
    else if(num_tokens >= 6 &&
       strcmp(tokens[0],"tc.math.add")==0 &&
       strcmp(tokens[1],"(")==0 &&
       strcmp(tokens[3],",")==0 &&
       strcmp(tokens[5],")")==0)
    {
        char a[64],b[64];
        strcpy(a,tokens[2]);
        strcpy(b,tokens[4]);
        normalize(a); normalize(b);
        load(a,"rax");
        load(b,"rbx");
        fprintf(out,"    add rax,rbx\n");
        store(a,"rax");
    }
    else if(num_tokens >= 6 &&
       strcmp(tokens[0],"tc.math.subtract")==0 &&
       strcmp(tokens[1],"(")==0 &&
       strcmp(tokens[3],",")==0 &&
       strcmp(tokens[5],")")==0)
    {
        char a[64],b[64];
        strcpy(a,tokens[2]);
        strcpy(b,tokens[4]);
        normalize(a); normalize(b);
        load(a,"rax");
        load(b,"rbx");
        fprintf(out,"    sub rax,rbx\n");
        store(a,"rax");
    }
    else if(num_tokens >= 6 &&
       strcmp(tokens[0],"tc.math.multiply")==0 &&
       strcmp(tokens[1],"(")==0 &&
       strcmp(tokens[3],",")==0 &&
       strcmp(tokens[5],")")==0)
    {
        char a[64],b[64];
        strcpy(a,tokens[2]);
        strcpy(b,tokens[4]);
        normalize(a); normalize(b);
        load(a,"rax");
        load(b,"rbx");
        fprintf(out,"    imul rax,rbx\n");
        store(a,"rax");
    }
    else if(num_tokens >= 6 &&
       strcmp(tokens[0],"tc.math.divide")==0 &&
       strcmp(tokens[1],"(")==0 &&
       strcmp(tokens[3],",")==0 &&
       strcmp(tokens[5],")")==0)
    {
        char a[64],b[64];
        strcpy(a,tokens[2]);
        strcpy(b,tokens[4]);
        normalize(a); normalize(b);
        load(a,"rax");
        fprintf(out,"    cqo\n");
        load(b,"rbx");
        fprintf(out,"    idiv rbx\n");
        store(a,"rax");
    }
    else {
        printf("Did not find %s in syntax (Core Dumped)\n", line);
        return_value = 1;
        return 1;
    }

    return 0;
}

static void compile(const char *in,const char *outf,const char *entry)
{
    FILE *f=fopen(in,"r");
    out=fopen(outf,"w");

    emit_header(entry);

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), f)) {
        if (is_comment(line)) {
            // Ta bort raden helt genom att nollställa den
            line[0] = '\0';
            continue;
        } else {
            compile_line(line);
        }
    }

    if(!is_init_called()) {
        printf("Warning: tc.init() was never called!\n");
        return;
    }

    if(!is_retinit_called()) {
        printf("Warning: tc.ret.init() was never called!\n");
        return;
    }

    emit_runtime();

    fclose(f);
    fclose(out);
}

void print_help() {
    printf("Usage: tc -i input.tc [-o output.asm] [-e entry_function] [-h]\n");
    printf("Options:\n");
    printf("  -i <file>     Input file to compile (required)\n");
    printf("  -o <file>     Output file (default: [Input File Name].asm)\n");
    printf("  -e <func>     Entry point function (default: main)\n");
    printf("  -s <size>     Stack buffer size for local variables\n");
    printf("  -h            Show this help message\n");
}

int main(int argc, char** argv) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *entry_point = "main"; // default

    if(argc < 2){
        print_help();
        return 1;
    }

    for(int i=1; i<argc; i++){
        if(strcmp(argv[i], "-i") == 0 && i+1 < argc){
            input_file = argv[++i];
        } else if(strcmp(argv[i], "-o") == 0 && i+1 < argc){
            output_file = argv[++i];
        } else if(strcmp(argv[i], "-e") == 0 && i+1 < argc){
            entry_point = argv[++i];
        } else if(strcmp(argv[i], "-h") == 0){
            print_help();
            return 0;
        } else if(strcmp(argv[i], "-vb") == 0 && i+1 < argc){
            stack_buffer_size = atoi(argv[++i]);

            if(stack_buffer_size <= 0){
                printf("Invalid buffer size\n");
                return 1;
            }
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            print_help();
            return 1;
        }
    }

    if(!input_file){
        printf("Error: input file required\n");
        print_help();
        return 1;
    }

    char out[256];
    if(output_file){
        snprintf(out, sizeof(out), "%s", output_file);
    } else {
        snprintf(out, 256, "%s.asm", input_file);
    }

    printf("Compiling: %s\nOutput: %s\nEntry: %s\n", input_file, out, entry_point);
    compile(input_file, out, entry_point);

    if(return_value == 1) return 1;
    return 0;
}
