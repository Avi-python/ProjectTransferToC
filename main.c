#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <math.h>

#include "MyHeader/extension.c"
#include "MyHeader/hash_2.c"
#include "MyHeader/queue_2.c"

#define BUFFER_SIZE 1024
#define MAXLISTEN 10

// 方法宣告
void *ListentForClient(void *);
void SystemMsg(char , char *);
void SendAll(char *);
void SendBack(char *, int);
void SendTo(char *, char *);
char *OnlineList(); 
void TryMatch();
void CreateRoom(char *, char *);
char *Enemy(char *);
char *GetRoomID();

// char **Split(char *, char, int);
// 打包線上使用者UserNameQueue裡面的名子成為一個字串(字元陣列)。

// int clientFd;
// 用來紀錄新進入的client，並在子線程等名稱送過來後做打包 放進去 Hashtable，但是有個弊端就是無法100%保證不會有錯名的情況 
// pthread_t* clientThread; 
// 全局物件宣告
int clientCnt = 0;
char *nameList[MAXLISTEN];
hashtab *Hashtable;
hashtab *UserToRoom; // 不知道這個做不做得起來
hashtab *Room;
Queue *WaitingQueue;
Queue *UserNameQueue;

typedef struct FdWithAddr
{
    struct sockaddr_in addr;
    int fd;
    int id;
    pthread_t *thread;
} FWA;

int main(int argc, char** argv)
{
    // Initiallize
    Hashtable = newhashtab(101);
    UserToRoom = newhashtab(101); // 不知道這個做不做得起來
    Room = newhashtab(101);
    WaitingQueue = newQueue();
    UserNameQueue = newQueue();
    //

    int sockSrv = socket(AF_INET, SOCK_STREAM, 0); // 可以把 0 換成 TCP 的 protocol
    if (sockSrv == -1) // 表示沒有建立成功
    {
        perror("Can not create socket");
        close(sockSrv);
        exit(EXIT_FAILURE); // = exit(1);
    }

    // 抓本機 ip

    struct ifreq ifr;
    char array[] = "wifi0";

    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
    if(ioctl(sockSrv, SIOCGIFADDR, &ifr) != 0)
    {
        perror("Error: ");
        exit(1);
    }
    //display result
    printf("IP Address is %s - %s\n" , array , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );

    struct sockaddr_in addrSrv;
    addrSrv.sin_family = AF_INET;
    inet_pton(AF_INET, inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr), &(addrSrv.sin_addr));

    // 設定port
    // int portNum;
    // scanf("Please enter port number: %d", &portNum); 

    addrSrv.sin_port = htons(4000);
    printf("Port Number is - %d.\n", 4000);

    if(-1 == bind(sockSrv, (const struct sockaddr *)&addrSrv, sizeof(struct sockaddr_in)))
    {
        perror("error bind failed");
        close(sockSrv);
        exit(EXIT_FAILURE);
    }

    if(-1 == listen(sockSrv, MAXLISTEN))
    {
        perror("error listen failed");
        close(sockSrv);
        exit(EXIT_FAILURE);
    }

    char sendbuf[BUFFER_SIZE] = "Hello I'm Tcp Server!";

    int sockClient = -1;
    struct sockaddr_in addrCli;
    int len = sizeof(struct sockaddr_in);
    
    while(true)
    { 
        printf("accepting..\n");
        sockClient = accept(sockSrv, (struct sockaddr *)&addrCli, &len);
        if( sockClient == -1 )
        {
            perror("error accept failed");
            close(sockClient);
            // close(sockSrv);
        }
        else
        {
            if(clientCnt <= MAXLISTEN) 
            {
                printf("Create a connection!\n");

                FWA *newClient = (FWA *)malloc(sizeof(FWA));
                
                newClient->addr = addrCli;
                newClient->fd = sockClient;
                newClient->id = clientCnt;
                newClient->thread = (pthread_t *)malloc(sizeof(pthread_t));
                
                pthread_create(newClient->thread, NULL, ListentForClient, newClient);
                clientCnt++;
            }
        }
    }

    // close(sockSrv); // 直接關掉不接聽聽了


    fgetc(stdin); // linux沒有 system("pause")
    return 0;
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

void *ListentForClient(void *arg) // 所有個疑問，為什麼不直接把 thread 和 socket 直接當包裝在引數裡面傳進去呢?
{
    FWA *tmp = (FWA *)arg;

    bool pthreadLife = true;
    char recvbuf[BUFFER_SIZE];
    int result;
    char *selfName;

    while( pthreadLife ) // 這邊就可以使用pthread寫
    {
        result = recv(tmp->fd, recvbuf, sizeof(recvbuf), 0);
        // char test[100] = "7Point";
        // send(tmp->fd, test, sizeof(test), 0);
        if(result > 0)
        {
            
            // char server_ip[16];
            // inet_ntop(AF_INET, &addrCli.sin_addr.s_addr, server_ip, 16);
            // printf("Client addr:%s port:%d\n", server_ip, addrCli.sin_port);

            // 預處理
            recvbuf[result] = '\0';

            char Cmd = recvbuf[0];
            char* Str = (char *)malloc(sizeof(char) * (result - 1)); 
            // 如果不malloc，我把Str的值存進去我的各個資料結構裡面，在它的有效範圍經過以後就會被釋放，我存進去的值就會跟著被釋放，因為只是存指針進去並沒有複製。
            strncpy(Str, recvbuf + 1, result - 1);
            Str[result - 1] = '\0';

            SystemMsg(Cmd, Str);

            switch(Cmd)
            {
                case '0':
                    {
                        Hashtable->Install(Hashtable, Str, tmp); 
                        selfName = Str;
                        WaitingQueue->Enqueue(WaitingQueue, Str);
                        UserNameQueue->Enqueue(UserNameQueue, Str);
                        // printf("GetSize:%d\n", UserNameQueue->GetSize(UserNameQueue));
                        SendAll(OnlineList());
                        TryMatch();                    
                        break;
                    }
                case '1':
                    {
                        char **splitter = Split(Str, '|', sizeof(Str) / sizeof(char));  
                        // printf("%s\n", recvbuf);
                        SendAll(recvbuf);
                        char systemMsg1[50] = "(public)";
                        strcat(systemMsg1, splitter[0]);
                        strcat(systemMsg1, " by:");
                        strcat(systemMsg1, splitter[1]);
                        SystemMsg('D', systemMsg1); // 'D' for Dialog 
                        break;
                    }
                case '/':
                    {
                        char **splitter = Split(Str, ' ', sizeof(Str) / sizeof(char));
                        char *anotherName = splitter[0];
                        char **splitter2 = Split(recvbuf + (CharArrLen(anotherName) + 2), '|', result);
                        if(Hashtable->lookup(Hashtable, anotherName) != NULL) 
                        {
                            char sendMsg[50] = "2";
                            strcat(sendMsg, splitter2[0]);
                            strcat(sendMsg, " by: ");
                            strcat(sendMsg, splitter2[1]);
                            SendTo(sendMsg, anotherName);
                        }
                        else
                        {
                            char sendMsg[50] = "7!Message cannot send, User is offline!";
                            SendBack(sendMsg, tmp->fd);
                        }
                        break;
                    }
                case '4':
                    {
                        char systemMsg1[50] = "Exchange";
                        strcat(systemMsg1, Str);
                        SystemMsg('P', systemMsg1);
                        char **SenderMsg = Split(Str, '|', sizeof(Str) / sizeof(char));
                        char *SenderName = SenderMsg[0];

                        char sendMsg[50] = "5";
                        strcat(sendMsg, "Wait");
                        SendTo(sendMsg, SenderName);

                        char sendMsg2[50] = "4";
                        strcat(sendMsg2, SenderMsg[1]);
                        SendTo(sendMsg2, Enemy(SenderName));
                        break;
                    }
                case '6':
                    {
                        char **SenderMsg = Split(Str, '|', sizeof(Str) / sizeof(char));
                        char *SenderName = SenderMsg[0];
                        if(Enemy(SenderName) != NULL) 
                        {
                            char sendMsg[50] = "6";
                            strcat(sendMsg, SenderMsg[1]);
                            SendTo(sendMsg, Enemy(SenderName));
                        }
                        else
                        {
                            char sendMsg[50] = "7!Enemy has leaved, cannot Send messages!";
                            SendTo(sendMsg, SenderName);
                        }
                        break;
                    }
                case '7':
                    {
                        char sendMsg[50] = "7(Process)Push you to the waiting Queue!!!";
                        SendTo(sendMsg, Str);
                        WaitingQueue->Enqueue(WaitingQueue, Str);
                        TryMatch();
                        break;
                    }
                case '8':
                    {
                        char systemMsg1[50] = "\0";
                        strcat(systemMsg1, Str);
                        strcat(systemMsg1, "Win a game!!");
                        SystemMsg('P', systemMsg1);
                        SendTo("8", Enemy(Str));

                        struct nlist *out = UserToRoom->lookup(UserToRoom, Str);
                        char *seed = (char *)out->data;
                        UserToRoom->Remove(UserToRoom, Enemy(Str));
                        UserToRoom->Remove(UserToRoom, Str);
                        Room->Remove(Room, seed);

                        char systemMsg2[50] = "Close a Room(seed:";
                        strcat(systemMsg2, seed);
                        strcat(systemMsg2, ")");
                        SystemMsg('P', systemMsg2);
                        break;
                    }
                case '9':
                    {
                        char systemMsg1[50] = "\0";
                        strcat(systemMsg1, Str);
                        strcat(systemMsg1, " User AFK");
                        SystemMsg('P', systemMsg1);

                        Hashtable->Remove(Hashtable, Str);

                        if(WaitingQueue->GetSize(WaitingQueue) != 0 && strcmp((char *)WaitingQueue->head->data, Str) == 0)
                        {
                            WaitingQueue->Dequeue(WaitingQueue);
                        }
                        else if(Enemy(Str) != NULL)
                        {
                            SendTo("9", Enemy(Str));
                            WaitingQueue->Enqueue(WaitingQueue, Enemy(Str));
                            struct nlist *out = UserToRoom->lookup(UserToRoom, Str);
                            char *seed = (char *)out->data;
                            UserToRoom->Remove(UserToRoom, Enemy(Str));
                            UserToRoom->Remove(UserToRoom, Str);
                            Room->Remove(Room, seed);

                            char systemMsg2[50] = "Close a Room(seed:";
                            strcat(systemMsg2, seed);
                            strcat(systemMsg2, ")");
                            SystemMsg('P', systemMsg2);

                            free(seed);
                        }
                        UserNameQueue->RemoveStr(UserNameQueue, Str);
                        // printf("size :%d\n", UserNameQueue->GetSize(UserNameQueue)); 
                        // ShowQueue(UserNameQueue);
                        char tmp1[50];
                        char *tmp2 = OnlineList();
                        strncpy(tmp1, tmp2, CharArrLen(tmp2));
                        tmp1[CharArrLen(tmp2)] = '\0';
                        // printf("tmp1:%s\n", tmp1);
                        // printf("size :%d\n", UserNameQueue->GetSize(UserNameQueue));
                        // ShowQueue(UserNameQueue);
                        SendAll(tmp1);
                        usleep(100000);
                        // ShowQueue(UserNameQueue);
                        TryMatch();

                        pthreadLife = false;
                        break;
                    }
                default:
                    {
                        SystemMsg('E', "!!!Unknown Cmd Type!!!");
                        break;
                    }
            }
            // SystemMsg(Cmd, Str);
        }
        else
        {
            SystemMsg('E', "!!!Server Wrong Please Check!!!");
            pthreadLife = false;
        }
        result = 0;
    }
    // printf("%s\n", selfName);
    if(0 > close(tmp->fd))
    {
        perror("Error: ");
        exit(1);
    }
    free(tmp);
    tmp = NULL;
    SystemMsg('P', "Close One Client Thread");
    pthread_exit(NULL);
}

void SystemMsg(char type, char *msg)
{
    printf("%c:%s\n", type, msg);
}

char * OnlineList()
{
    int size = UserNameQueue->GetSize(UserNameQueue);
    char* output = calloc(100, sizeof(char));
    output[0] = 'L';
    char *out;
    char tmp[20];
    int length;
    // ShowQueue(UserNameQueue);
    for(int i = 0; i < size; i++)
    {
        out = (char *)UserNameQueue->Dequeue(UserNameQueue);
        length = CharArrLen(out);
        strncpy(tmp, out, length);
        strncat(output, tmp, length);
        if(i < size - 1) strcat(output, ",");
        char *copy = calloc(length + 1, sizeof(char));
        // strncpy(copy, tmp, length);
        // copy[length] = '\0';
        // printf("copy:%s\n", copy);
        UserNameQueue->Enqueue(UserNameQueue, out);
        // ShowQueue(UserNameQueue);
    }
    // ShowQueue(UserNameQueue);
    // SystemMsg('\0', output);
    return output;
}

void SendTo(char *Str1, char* User)
{
    struct nlist *out = Hashtable->lookup(Hashtable, User);
    FWA* fwa = (FWA *)out->data;

    int size = CharArrLen(Str1);

    int sockClient = fwa->fd;
    send(sockClient, Str1, size * sizeof(char), 0);
}

void SendBack(char *Str1, int sockFd)
{
    send(sockFd, Str1, CharArrLen(Str1) * sizeof(char), 0); 
}

void SendAll(char *input)
{
    int n = UserNameQueue->GetSize(UserNameQueue);
    
    char *out;

    int size = CharArrLen(input); // 不可以加一，要剛剛好，無須管最後面的 '\0'。

    for(int i = 0; i < n; i++)
    {
        out = (char *)UserNameQueue->Dequeue(UserNameQueue);
        struct nlist *tmp = Hashtable->lookup(Hashtable, out);
        FWA *fwa = (FWA *)(tmp->data);
        // printf("input:%s\n", input);
        send(fwa->fd, input, size * sizeof(char), 0);
        UserNameQueue->Enqueue(UserNameQueue, out);
    }
}

char *Enemy(char *user)
{
    if(UserToRoom->lookup(UserToRoom, user) != NULL)
    {
        struct nlist * out = UserToRoom->lookup(UserToRoom, user);
        char * seed = (char *)(out->data);
        struct nlist * out2 = Room->lookup(Room, seed);
        char **userArr  = (char **)(out2->data);
        // printf("u1:%s, u2:%s\n", userArr[0], userArr[1]);
        if(!strcmp(userArr[0], user)) return userArr[1];
        else if(!strcmp(userArr[1], user)) return userArr[0];
        else
        {
            // 先試試看
            perror("Error");
            exit(1);
        }
    }
    else
    {
        return NULL;
    }
}

void TryMatch()
{
    if(WaitingQueue->GetSize(WaitingQueue) >= 2)
    {
        char *Player1 = WaitingQueue->Dequeue(WaitingQueue);
        char *Player2 = WaitingQueue->Dequeue(WaitingQueue);
        CreateRoom(Player1, Player2);

        // 決定先後
        srand(time(NULL));
        int x = rand() % 2;
        
        char send1buf[100] = "\0";
        char send2buf[100] = "\0";
        char systemMsg2[100] = "Send 001 to ";
        char systemMsg3[100] = "Send 002 to ";

        int n1 = CharArrLen(Player1);
        int n2 = CharArrLen(Player2);

        if(x) // x == 1
        {
            strcat(send1buf, "3001|");

            char tmp1[20];
            strncpy(tmp1, Player2, n2);
            tmp1[n2] = '\0';

            char tmp2[20];
            strncpy(tmp2, Player1, n1);
            tmp2[n1] = '\0';

            strcat(send1buf, tmp1);
            SystemMsg('T', send1buf);
            SendTo(send1buf, Player1);
            strcat(systemMsg2, tmp1);
            SystemMsg('P', systemMsg2);

            strcat(send2buf, "3002|");

            strcat(send2buf, tmp2);
            SystemMsg('T', send2buf);
            SendTo(send2buf, Player2);
            strcat(systemMsg3, tmp2);
            SystemMsg('P', systemMsg3);   
        }
        else
        {
            char tmp1[20];
            strncpy(tmp1, Player2, n2);
            tmp1[n2] = '\0';

            char tmp2[20];
            strncpy(tmp2, Player1, n1);
            tmp2[n1] = '\0';
            
            strcat(send2buf, "3002|");
            strcat(send2buf, tmp1);
            SystemMsg('T', send2buf);
            SendTo(send2buf, Player1);

            strcat(systemMsg3, tmp1);
            SystemMsg('P', systemMsg3); 

            strcat(send1buf, "3001|");
            strcat(send1buf, tmp2);
            SystemMsg('T', send1buf);
            SendTo(send1buf, Player2);

            strcat(systemMsg2, tmp2);
            SystemMsg('P', systemMsg2); 
        }

    }
}

void CreateRoom(char *user1, char *user2)
{
    char *roomID = GetRoomID();
    // 記得malloc
    char **room = (char **)malloc(sizeof(char *) * 2);
    room[0] = user1;
    UserToRoom->Install(UserToRoom, user1, roomID);
    UserToRoom->Install(UserToRoom, user2, roomID);
    room[1] = user2;
    //我可以想到的解法就是roomID改用字串，可以數字轉字串。
    Room->Install(Room, roomID, room);

    char systemMsg1[100] = "Create New Room to [";
    strcat(systemMsg1, user1);
    strcat(systemMsg1, ", ");
    strcat(systemMsg1, user2);
    strcat(systemMsg1, "]");
    strcat(systemMsg1, "(seed:");
    strcat(systemMsg1, roomID);
    strcat(systemMsg1, ")");
    SystemMsg('\0', systemMsg1);
}

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

