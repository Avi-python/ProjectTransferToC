
int CharArrLen(char *input)
{
    int cnt = 0;
    for(char *it = input; *it != '\0'; it++)
    {
        cnt++;
    }
    return cnt;
}

int DigitCounter(long input)
{
    int cnt = 0;
    long tmp = input;
    while(tmp / 10 != 0)
    {
        cnt++;
        tmp /= 10;
    }
    if(tmp % 10 != 0) cnt++;
    return cnt;
}

// 一個非常簡單的splitter，只會碰到第一個就切割而已哇哈哈。
char **Split(char * Str1, char key, int size)
{
    char *prefix = calloc(size, sizeof(char));
    char *postfix = calloc(size, sizeof(char));
    char **charArr = calloc(2, sizeof(char *));
    
    int index = 0;
    while(true)
    {
        if(Str1[index] == key) break;
        prefix[index] = Str1[index];
        index++;
    }
    int postindex = 0;
    index++;
    while(true)
    {
        if(Str1[index] == '\0') break;
        postfix[postindex] = Str1[index];
        postindex++;
        index++;
    }
    charArr[0] = prefix;
    charArr[1] = postfix;
    return charArr;
}