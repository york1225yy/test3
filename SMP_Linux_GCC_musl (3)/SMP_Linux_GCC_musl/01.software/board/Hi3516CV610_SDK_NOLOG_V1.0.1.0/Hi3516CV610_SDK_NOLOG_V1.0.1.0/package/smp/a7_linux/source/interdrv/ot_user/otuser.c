/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_user.h"
#include "securec.h"

#if AO_NOTIFY

static void ot_user_notify_ao_event(int ao_dev)
{
    switch (ao_dev) {
        case 0: { /* 0 device */
            /* do something */
            break;
        }
        case 1: { /* 1 device */
            /* do something */
            break;
        }
        default: {
            break;
        }
    }
}

#endif

int __init ot_user_init(void)
{
#if AO_NOTIFY
    {
        ot_ao_export_symbol *ao_export_symbol = NULL;
        ot_ao_export_callback ao_export_callback;
        (void)memset_s(&ao_export_callback, sizeof(ot_ao_export_callback), 0, sizeof(ot_ao_export_callback));

        ao_export_symbol = ot_ao_get_export_symbol();
        if (ao_export_symbol != NULL) {
            if (ao_export_symbol->register_export_callback != NULL) {
                ao_export_callback.ao_notify = ot_user_notify_ao_event;
                ao_export_symbol->register_export_callback(&ao_export_callback);
            }
        }
    }
#endif
    printk("load ot_user ....OK!\n");
    return 0;
}

void __exit ot_user_exit(void)
{
#if AO_NOTIFY
    {
        ot_ao_export_symbol *ao_export_symbol = NULL;
        ot_ao_export_callback ao_export_callback;

        ao_export_symbol = ot_ao_get_export_symbol();
        if (ao_export_symbol != NULL) {
            if (ao_export_symbol->register_export_callback != NULL) {
                (void)memset_s(&ao_export_callback, sizeof(ot_ao_export_callback), 0, sizeof(ot_ao_export_callback));
                ao_export_symbol->register_export_callback(&ao_export_callback);
            }
        }
    }
#endif

    printk("unload ot_user ....OK!\n");
}

#ifndef __LITEOS__
module_init(ot_user_init);
module_exit(ot_user_exit);
#endif

MODULE_LICENSE("GPL");
