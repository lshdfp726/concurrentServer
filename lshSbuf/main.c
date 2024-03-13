#include "lshSbuf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    /* code */
    lshSbuf sp = {0};
    lshSbuf_init(&sp, 2, "test");

    lshSbuf_insert(&sp, 999);

    lshSbuf_remove(&sp);
    return 0;

}
