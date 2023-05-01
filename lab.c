#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <math.h>
#include "MyHeader/hash_2.c"
#include "MyHeader/queue_2.c"

int CharArrLen(char *input)
{
    int cnt = 0;
    for(char *it = input; *it != '\0'; it++)
    {
        cnt++;
    }
    return cnt;
}

void ShowQueue(Queue *q)
{
    Node *cur = q->head;
    while(cur != NULL)
    {
        printf("%s, ", (char *)cur->data);
        cur = cur->next;
    }
    printf("\n");
}

typedef struct 
{
    char *name;
    int num;
} data;

Queue* UserNameQueue;

char * OnlineList()
{
    int size = UserNameQueue->GetSize(UserNameQueue);
    char* output = calloc(100, sizeof(char));
    output[0] = 'L';
    char *out;
    char tmp[20];
    int length;
    ShowQueue(UserNameQueue);
    for(int i = 0; i < size; i++)
    {
        out = (char *)UserNameQueue->Dequeue(UserNameQueue);
        length = CharArrLen(out);
        strncpy(tmp, out, length);
        strncat(output, tmp, length);
        if(i < size - 1) strcat(output, ",");
        UserNameQueue->Enqueue(UserNameQueue, out);
    }
    ShowQueue(UserNameQueue);
    // SystemMsg('\0', output);
    return output;
}

int main()
{
    char name1[10] = "D1";
    char name2[10] = "D2";
    char name3[10] = "D3";
    strcat(name1, name2);
    printf("%s\n", name1);
    printf("%s\n", name2);
    name2[0] = 'C';
    printf("name2:%s\n", name2);
    printf("%s\n", name1);
    
    // OnlineList();
    return 0;
}