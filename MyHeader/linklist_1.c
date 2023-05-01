
typedef struct linklist_form linklist;

typedef bool (*_Insert)(linklist *, char *);
typedef void (*_Delete)(linklist *, char *);
typedef int (*_Length)(linklist *);

typedef struct node_form
{
    node* next;
    char* name;
} node;

struct linklist_form
{
    node* head;
    _Insert Insert;
    _Delete Delete;
    _Length Length;
};

linklist *newlinklist()
{
    linklist *l = (linklist *)malloc(sizeof(linklist));
    l->head = NULL;
    
}

bool Insert_Impl(listlist *l, char *input)
{
    if(!l->Length_Impl(l))
    {
        l->head = (node *)malloc(sizeof(node));
        l->head->name = input;
        l->head->next = NULL;
    }
}

void Delete_Impl(linklist *l, char *input)
{

}

int Length_Impl(linklist *l)
{

}