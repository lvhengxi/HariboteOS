#include <stdio.h>

#include "api.h"
#include "console.h"
#include "task.h"

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx,
             int eax) {
  int ds_base = *((int *)0x0fe8);
  struct Task *task = task_now();
  struct Console *cons = (struct Console *)*((int *)0x0fec);
  char s[12];

  if (edx == 1) {
    cons_putchar(cons, eax & 0xff, 1);
  } else if (edx == 2) {
    cons_putstr(cons, (char *)ebx + ds_base);
  } else if (edx == 3) {
    cons_putnstr(cons, (char *)ebx + ds_base, ecx);
  } else if (edx == 4) {
    return &(task->tss.esp0);
  }

  // } else if (edx == 123456789) {
  //   *((char *) 0x00102600) = 0;
  // }

  return 0;
}
