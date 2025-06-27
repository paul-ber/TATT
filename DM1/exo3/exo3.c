int isnum(char* s)
{
    unsigned int i = 0;
    for (i = 0; i < strlen(s); ++i)
        if (!isdigit(s[i]))
            return 0;
    return 1;
}

void ispar(char* s)
{
    unsigned int i = -1;
    char sentence[64];
    i = atoi(s) % 2;
    strcpy(sentence,s);
    if (i == 0)
        printf("%s est un nombre pair\n", s);
    else if (i == 1)
        strcat("%s est un nombre impair\n", s);
    else
        printf("Impossible de d´eterminer la parit´e\n");
}

int main(int argc,
        char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s number\n", argv[0]);
        return 1;
    }
    if (!isnum(argv[1]))
    {
        printf("Ceci n’est pas un nombre\n");
        return 2;
    }
    ispar(argv[1]);
    return 0;
}
