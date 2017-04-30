//  Created by Ahan Shabanov on 21.02.17.
//  Copyright © 2017 Akhmedkhan Shabanov. All rights reserved.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "sarr_lib.h"

int compar1(const void* d1, const void* d2)
{
    if ((d1 == NULL) || (d2==NULL))
    {
        perror("NULL ptr");
        return -1;
    }

    int v1 = *(int*)d1;
    int v2 = *(int*)d2;
    if (v1 > v2)
    {
        return 1;
    }
    else
    if (v1<v2)
        return (-1);
    else
    {
         return 0;
    }
}

int compar2(const void* d1, const void* d2)
{
    char* val1 = *(char**)d1;
    char* val2 = *(char**)d2;

    return strcmp(val1, val2);

}

int main(int argc, const char * argv[])
{
    s_array* mass1, *mass2;
    iterator* it, *it_c;
    int a;
    char* t;

    mass1 = creat_s_array(3, sizeof(int), compar1);

    a=5;
    add(mass1, &a);
    a=1;
    add(mass1,&a);
    a=4;
    add(mass1, &a);
    a=2;
    add(mass1, &a);
    a=4;
    del(mass1, &a);

    it = creat_iter(mass1);

    for (it_begin(it); !is_end(it); next(it))
    {
        printf("%d\n",*(int*)get(it));
    }


    destr_s_array(mass1);
    destr_iter(it);


    mass2 = creat_s_array(5, sizeof(char*), compar2);
    t = "dsd";
    add(mass2, &t);
    t = "acxz";
    add(mass2, &t);

    it_c = creat_iter(mass2);
    for (it_begin(it_c); !is_end(it_c); next(it_c))
    {
        printf("%s\n",*(char**)get(it_c));
    }
    destr_iter(it_c);
    destr_s_array(mass2);

}
