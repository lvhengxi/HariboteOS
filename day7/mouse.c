#include "fifo.h"
#include "io.h"
#include "keyboard.h"
#include "mouse.h"

struct FIFO8 mousefifo;
unsigned char mousebuf[MOUSE_FIFO_BUF_SIZE];

/*鼠标激活，往键盘控制器写入0xd4下一个数据就会给到鼠标，然后鼠标会返回0xfa*/
void enable_mouse(void) {
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
}
