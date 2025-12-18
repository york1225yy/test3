/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description:
 * Author: pqtool team
 * Create: 2023/6/1
 */

#include <stdio.h>
#include <signal.h>
#include "ot_main.h"
volatile sig_atomic_t g_aqStream = 1;
int main(int argc, char *argv[])
{
    return AqControlMain(argc, argv);
}
