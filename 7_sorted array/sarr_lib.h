typedef struct s_array s_array;
typedef struct iterrator iterator;

struct s_array
{
    void* array;
    int size;
    int size_el;
    int last_el;
    int (*cmp)(const void*, const void*);
};

struct iterrator
{
    s_array* ptr;
    int index;

};

s_array* creat_s_array ( int size, int size_el ,int (*cmp)(const void*, const void*) );
int add (s_array* mass, void* data);
int find (s_array* mass, void* data);
int del (s_array* mass, void* data);
int binsearch (void* put, s_array* mass, int (*cmp)(const void*, const void*));
int destr_s_array(s_array*);

void* wrong_malloc(int size);

//iterator
iterator* creat_iter(s_array* mass);
int next(iterator*);
int previous(iterator*);
void* get(iterator*);
int is_begin(iterator*);
int is_end(iterator*);
int it_begin(iterator*);
int destr_iter(iterator*);
