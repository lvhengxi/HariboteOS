#include <stdio.h>

#include "bootpack.h"
#include "desctbl.h"
#include "fifo.h"
#include "graphic.h"
#include "int.h"
#include "io.h"
#include "keyboard.h"
#include "mem.h"
#include "mouse.h"

int main(void) {
  struct BootInfo *binfo = (struct BootInfo *)ADR_BOOTINFO;
  struct MemMan *memman = (struct MemMan *)MEMMAN_ADDR;
  struct MouseDec mdec;
  char s[40], mcursor[256];
  unsigned char data;
  unsigned int memtotal;

  init_gdtidt();
  init_pic(); // GDT/IDT完成初始化，开放CPU中断

  io_sti();

  fifo8_init(&keyfifo, KEY_FIFO_BUF_SIZE, keybuf);
  fifo8_init(&mousefifo, MOUSE_FIFO_BUF_SIZE, mousebuf);

  io_out8(PIC0_IMR, 0xf9); // 开放PIC1以及键盘中断
  io_out8(PIC1_IMR, 0xef); // 开放鼠标中断

  init_keyboard();

  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);
  put_block8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%d, %d)", mx, my);
  put_fonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

  enable_mouse(&mdec);

  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000); // 0x00001000 ~ 0x0009efff
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  sprintf(s, "memory %dMB, free: %dKB, use percentage", memtotal / (1024 * 1024),
          memman_total(memman) / 1024);
  put_fonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

  for (;;) {
    io_cli();
    if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
      io_stihlt();
    } else {
      if (fifo8_status(&keyfifo)) {
        data = (unsigned char)fifo8_get(&keyfifo);

        io_sti();
        sprintf(s, "%02X", data);
        box_fill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
        put_fonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
      } else if (fifo8_status(&mousefifo)) {
        data = (unsigned char)fifo8_get(&mousefifo);

        io_sti();

        if (mouse_decode(&mdec, data)) {
          sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);

          if ((mdec.btn & 0x01)) {
            s[1] = 'L';
          }

          if ((mdec.btn & 0x02)) {
            s[3] = 'R';
          }

          if ((mdec.btn & 0x04)) {
            s[2] = 'C';
          }

          box_fill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16,
                    32 + 15 * 8 - 1, 31);
          put_fonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
          box_fill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15,
                    my + 15);

          mx += mdec.x;
          my += mdec.y;

          if (mx < 0) {
            mx = 0;
          }
          if (my < 0) {
            my = 0;
          }
          if (mx > binfo->scrnx - 16) {
            mx = binfo->scrnx - 16;
          }
          if (my > binfo->scrny - 16) {
            my = binfo->scrny - 16;
          }

          sprintf(s, "(%3d, %3d)", mx, my);
          box_fill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79,
                    15); // 隐藏坐标
          put_fonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF,
                         s); // 显示坐标
          put_block8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor,
                       16); // 描画鼠标
        }
      }
    }
  }

  return 0;
}
