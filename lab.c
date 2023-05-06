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
#include "MyHeader/extension.c"

char *GetRoomID()
{
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long seed =  (te.tv_sec % 1000000) * 1000000 + te.tv_usec; // calculate milliseconds
    
    int n = DigitCounter(seed);
    int i;
    char *numberArray = calloc(n, sizeof(char));
    for (i = n-1; i >= 0; --i, seed /= 10)
    {
        numberArray[i] = (seed % 10) + '0';
    }
    return numberArray;
}


int main()
{
    printf("ID:%s", GetRoomID());
    
    // OnlineList();
    return 0;
}