
// #define HASHSIZE 101
// static struct nlist *hashtab[HASHSIZE]; /* pointer table */
// static 會導致我在其他資料夾無法訪問
// 我的data就是直接把pointer傳進去

typedef struct hashtab_def hashtab; 

struct nlist { /* table entry: */
    struct nlist *next; /* next entry in chain */
    char *name; /* defined name */
    void *data; /* replacement text */
};

typedef unsigned (*_hash)(hashtab *, char *);
typedef struct nlist* (*_lookup)(hashtab *, char *);
typedef char* (*_MakeDupStr)(char *);
typedef struct nlist* (*_Install)(hashtab *, char *, void *);
typedef void (*_Remove)(hashtab *, char *);


struct hashtab_def
{
    int HASHSIZE;
    struct nlist** table;
    _hash hash;
    _lookup lookup;
    _MakeDupStr MakeDupStr;
    _Install Install;
    _Remove Remove;
};

unsigned hash_Impl(hashtab * , char* );
struct nlist *lookup_Impl(hashtab *, char *);
char *MakeDupStr_Impl(char *);
struct nlist *Install_Impl(hashtab *, char *,  void *);
void Remove_Impl(hashtab *, char *);

hashtab* newhashtab(int size)
{
    hashtab *newtab = (hashtab *)malloc(sizeof(hashtab));
    
    newtab->HASHSIZE = size;
    newtab->table = calloc(size, sizeof(struct nlist *));
    newtab->hash = hash_Impl;
    newtab->lookup = lookup_Impl;
    newtab->MakeDupStr = MakeDupStr_Impl;
    newtab->Install = Install_Impl;
    newtab->Remove = Remove_Impl;
    
    return newtab;
}

// typedef struct data_form
// {
//     int num;
//     char name[20];
// } data;

/* hash: form hash value for string s */
unsigned hash_Impl(hashtab* h, char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % h->HASHSIZE;
}

/* lookup: look for s in hashtab */
struct nlist *lookup_Impl(hashtab* h, char *s)
{
    struct nlist *np;
    for (np = h->table[h->hash(h, s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}

void Remove_Impl(hashtab* h, char *s)
{
    struct nlist *cur = NULL; // 因為我要remove這邊做的事情跟link list一樣
    struct nlist *pre = NULL;
    int cnt = 0;
    for (cur = h->table[h->hash(h, s)]; cur != NULL; pre = cur, cur = cur->next)
    {    
        if (strcmp(s, cur->name) == 0)
        {
            if(cnt == 0)
            {
                h->table[h->hash(h, s)] = cur->next;
            }
            else
            {
                pre->next = cur->next;
            }
            // free(cur->data); 好像是 void pointer 所以free不了，應該說在同一個scope都沒有malloc，所以也不知道大小
            // 況且，我們是存著別人的地址，其實不可以free掉，可能會free到別人的資料。
            // free(cur->name);
            free(cur);
            cur = NULL;
            return;
        }
        cnt++;
    }
    // 因為按理來說不會有找不到的問題啦
    perror("Error: (Remove Not Found)"); /* not found */ 
    exit(1);
}

/* install: put (name, data) in hashtab */
struct nlist *Install_Impl(hashtab* h, char *name, void *input)
{
    struct nlist *np;
    unsigned hashval;
    if ((np = h->lookup(h, name)) == NULL) 
    { /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = MakeDupStr_Impl(name)) == NULL)
          return NULL;
        hashval = h->hash(h, name);
        np->next = h->table[hashval];
        h->table[hashval] = np;
    } 
    else /* already there */
    {
        free((void *) np->data); /*free previous data */
    }
    
    // 因為 void 所以更改了一些狀況
    // if ((np->data = MakeDupStr_Impl(data)) == NULL)
    //    return NULL;
    np->data = input;
    return np;
}

char *MakeDupStr_Impl(char *s) /* make a duplicate of s */
{
    char *p; // 繼續思考怎麼使用 void
    p = (char *) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
       strcpy(p, s);
    return p;
}