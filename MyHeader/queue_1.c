#include<stdio.h>

typedef struct queue Queue;
typedef bool (*_emptyFun_p)(Queue *);
typedef void (*_enqueueFun_p)(Queue *, char *);
typedef char * (*_dequeueFun_p)(Queue *);

Queue* newQueue();
bool Empty_Impl(Queue *);
void Enqueue_Impl(Queue *, char *);
char* Dequeue_Impl(Queue *);

typedef struct node
{
    struct node *next;
    char *name;
} Node;

typedef struct queue
{
    Node *head;
    Node *tail;
    _emptyFun_p Empty;
    _enqueueFun_p Enqueue;
    _dequeueFun_p Dequeue;
} Queue;


bool Empty_Impl(Queue *q)
{
    if(q->head == NULL) { return true; }
    
    return false;
}

void Enqueue_Impl(Queue *q, char *input)
{ 
    Node* n = (Node *)malloc(sizeof(Node));
    
    n->next = NULL;
    n->name = input;
    
    if(q->Empty(q)) { q->head = n; q->tail = n; return; }
    
    q->tail->next = n;
    q->tail = q->tail->next;
}

char* Dequeue_Impl(Queue *q)
{
    if(q->Empty(q)) { return NULL; }
    
    int n = 0;
    Node *dl = q->head;
    for(char *it = dl->name; *it != '\0'; it++) n++;
    char *result = (char *)malloc(sizeof(char) * (n + 1));
    strncpy(result, dl->name, n);
    
    result[n] = '\0';
    
    q->head = q->head->next;
    
    free(dl);
    
    return result;
}

Queue* newQueue()
{
    Queue* Q = (Queue *)malloc(sizeof(Queue));
    Q->head = NULL;
    Q->tail = NULL;
    Q->Empty = Empty_Impl;
    Q->Enqueue = Enqueue_Impl;
    Q->Dequeue = Dequeue_Impl;
    return Q;
}