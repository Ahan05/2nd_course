#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sarr_lib.h"

void* wrong_malloc(int size)
{
    static int schet = 1;
    if ((schet % 7) == 0)
    {
        schet++;
        return NULL;
    }
    else
    {
        schet++;
        return malloc(size);
    }
}

s_array* creat_s_array( int size, int size_el ,int (*cmp)(const void*, const void*) )
{
    if ( (cmp == NULL)|| (size < 0) || (size_el < 0) )
    {
        errno = EINVAL;
        return NULL;
    }
    s_array* array;
    array = (s_array*)wrong_malloc(sizeof(s_array));
    if (array == NULL)
    {
        return NULL;
    }

    array->size = size;
    array->size_el = size_el;
    array->last_el = -1;
    array->cmp = cmp;
    array->array = (void*)wrong_malloc(size * size_el);
    if (array->array == NULL)
    {
        free(array);
        return NULL;
    }

    return array;
}

int binsearch (void* put, s_array* mass, int (*cmp)(const void*, const void*))
{
    int L = 0, R = mass->last_el;
    int m = 0;
    int cmp_res;

    if ((mass->array == NULL) || (put == NULL) || (cmp == NULL))
        return -1;

    while ( L <= R )
    {
        m = (L + R) / 2;
        cmp_res = cmp(put, mass->array + m*mass->size_el);
        if ( cmp_res == 0 )
            return m;
        else
            if ( cmp_res<0 )
                R = m-1;
            else
                L = m + 1;
    }
    return L;

}

int add (s_array* mass, void* data)
{
    int ind;
    int i;

    if ((mass == NULL) || (data == NULL))
    {
        errno = EINVAL;
        return -1;
    }

    if ((mass->last_el+1) == (mass->size))
    {
        mass->size++;
        mass->array = (void*)realloc(mass->array, mass->size * mass->size_el);
    }
    if (mass->last_el < 0)
        ind = 0;
    else
        ind = binsearch(data, mass, mass->cmp );
    mass->last_el++;


    for ( i = mass->last_el; i>ind; i--)
    {
        memcpy(mass->array + i*mass->size_el, mass->array+(i-1)*mass->size_el, mass->size_el);
    }
    memcpy(mass->array + ind*(mass->size_el), data, mass->size_el);

    return 1;
}

int find (s_array* mass, void* data)
{
    int index;
    index = binsearch(data,mass , mass->cmp);

    if ( (index <=mass->last_el ) && (memcmp(data, mass->array+index*(mass->size_el), mass->size_el) == 0 ))
        return index;
    return -1;
}

int del (s_array* mass, void* data)
{
    int index;
    int i;

    if ((mass == NULL) || (data == NULL))
    {
        errno = EINVAL;
        return -1;
    }

    index = find(mass, data);

    if (index >= 0)
    {
        for (i=index; i<mass->last_el; i++)
        {
            memcpy(mass->array+i*mass->size_el, mass->array+(i+1)*mass->size_el, mass->size_el);
        }
        mass->last_el--;
    }
    else
    {
        return -1;
    }
    return 1;
}

int destr_s_array(s_array* mass)
{
    if (mass == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    free(mass->array);
    free(mass);
    return 1;
}

//==================================================
// iterator
iterator* creat_iter(s_array* mass)
{
    iterator* it;

    if (mass == NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    it = (iterator*)wrong_malloc(sizeof(iterator));

    if (it == NULL)
        return NULL;

    it->ptr = mass;
    it->index = 0;
    return it;
}

int next(iterator* it)
{
    if (it == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    it->index++;
    return 1;
}

int previous(iterator* it)
{
    if (it == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    it->index--;
    return 1;
}

void* get(iterator* it)
{
    if ((it == NULL)|| (it->index<0) || (it->index >= it->ptr->size) || (it->index > it->ptr->last_el))
    {
        errno = EINVAL;
        return NULL;
    }
    else
        return it->ptr->array+(it->index)*(it->ptr->size_el);
}


int is_end(iterator*it)
{
    if (it == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    if (it->index == it->ptr->last_el+1)
        return 1;
    else return 0;
}

int it_begin(iterator* it)
{
    if (it == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    it->index=0;
    return 1;
}

int destr_iter(iterator*it)
{
    if (it == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    it->index = -1;
    it->ptr = NULL;
    free(it);
    return 1;
}
