/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description:
 * Author: pqtool team
 * Create: 2023/6/1
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

extern int PQControlMain(int argc, char **argv);
static pthread_t g_threadId = 0;

static void *FunPQControl(void *param)
{
    int argc = 0;
    char **argv = NULL;
    PQControlMain(argc, argv);
    return NULL;
}

int main(int argc, char *argv[])
{
    int ret = pthread_create(&g_threadId, NULL, FunPQControl, NULL);
    if (ret != 0) {
        printf("FunPQControl failed!!!\n");
        return -1;
    }

    printf("PQControlMain start!!!\n");

    /*******************************/
    // do something
    printf("do something.\n");
    /*******************************/

    int argNum = 2;
    char *args[] = {"", "stop"};
    ret = PQControlMain(argNum, args);
    if (ret != 0) {
        printf("PQControlMain failed !!!\n");
    }

    pthread_join(g_threadId, NULL);
    g_threadId = 0;
    printf("PQControlMain stop!!!\n");
    return 0;
}
