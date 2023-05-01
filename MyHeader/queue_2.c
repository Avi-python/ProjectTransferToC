#include<stdio.h>

// 可以Queue進去任何的物件

typedef struct queue Queue;
typedef bool (*_emptyFun_p)(Queue *);
typedef bool (*_enqueueFun_p)(Queue *, void *);
typedef void * (*_dequeueFun_p)(Queue *);
typedef int (*_GetSize_p)(Queue *);
typedef void (*_RemoveStr)(Queue *, char *);

Queue* newQueue();
bool Empty_Impl(Queue *);
bool Enqueue_Impl(Queue *, void *);
void* Dequeue_Impl(Queue *);
int GetSize_Impl(Queue *);
void RemoveStr_Impl(Queue *, char *);

typedef struct node
{
    struct node *next;
    void *data;
} Node;

typedef struct queue
{
    Node *head;
    Node *tail;
    _emptyFun_p Empty;
    _enqueueFun_p Enqueue;
    _dequeueFun_p Dequeue;
    _GetSize_p GetSize;
    _RemoveStr RemoveStr;
} Queue;

Queue* newQueue()
{
    Queue* Q = (Queue *)malloc(sizeof(Queue));
    Q->head = NULL;
    Q->tail = NULL;
    Q->Empty = Empty_Impl;
    Q->Enqueue = Enqueue_Impl;
    Q->Dequeue = Dequeue_Impl;
    Q->GetSize = GetSize_Impl;
    Q->RemoveStr = RemoveStr_Impl;
    return Q;
}

bool Empty_Impl(Queue *q)
{
    if(q->head == NULL) { return true; }
    
    return false;
}

bool Enqueue_Impl(Queue *q, void *input)
{ 
    Node* n = (Node *)malloc(sizeof(Node));

    if(n == NULL)
    {
        perror("Error:");
        return false;
    }
    
    n->next = NULL;
    n->data = input;
    
    if(q->Empty(q)) { q->head = n; q->tail = n; return true; }
    
    q->tail->next = n;
    q->tail = q->tail->next;
    return true;
}

void* Dequeue_Impl(Queue *q)
{
    if(q->Empty(q)) { return NULL; }
    
    // for "char *"
    // int n = 0;
    // Node *dl = q->head;
    // for(char *it = dl->name; *it != '\0'; it++) n++;
    // char *result = (char *)malloc(sizeof(char) * (n + 1));
    // strncpy(result, dl->name, n);
    // result[n] = '\0';
    
    Node *tmp = q->head;
    void *result = tmp->data;
    
    q->head = q->head->next;
    
    free(tmp);
    
    return result;
}

int GetSize_Impl(Queue *q)
{
    Node *cur = q->head;
    int cnt = 0;
    while(cur != NULL)
    {
        cur = cur->next;
        cnt++;
    }
    return cnt;
}

void RemoveStr_Impl(Queue *q, char *input)
{
    Node *cur = q->head, *pre;
    int cnt = 0;
    while(cur != NULL)
    {
        if(!strcmp(cur->data, input))
        {
            if(cnt == 0) q->head = q->head->next;
            else         pre->next = cur->next;
            
            free(cur);
            return;
        }
        cnt++;
        pre = cur;
        cur = cur->next;
    }
}

