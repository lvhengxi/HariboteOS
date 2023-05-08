#include "desctbl.h"
#include "int.h"

void init_gdtidt(void) {
  /*在这里操作系统直接在内存中开地址当作表，nbplus666，真就把地址当作指针来用*/
  struct SegmentDescriptor *gdt = (struct SegmentDescriptor *)ADR_GDT;
  struct GateDescriptor *idt = (struct GateDescriptor *)ADR_IDT;

  for (int i = 0; i <= LIMIT_GDT / 8; i++) {
    set_segmdesc(gdt + i, 0, 0, 0);
  }
  /*这个大小是全局，表示该段可以访问整个内存空间*/
  set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
  /**/
  set_segmdesc(gdt + 2, LIMIT_BOOTPACK, ADR_BOOTPACK, AR_CODE32_ER);
  load_gdtr(LIMIT_GDT, ADR_GDT);

  for (int i = 0; i <= LIMIT_IDT / 8; i++) {
    set_gatedesc(idt + i, 0, 0, 0);
  }
  load_idtr(LIMIT_IDT, ADR_IDT);
  /*2*8是2<<3。IDT 表中的每个中断描述符需要占用 8 个字节（64 位），其中最后三
  个字节必须设置为 0。这是因为在 CPU 执行跳转指令时，会将目标地址的最后 3 位作
  为段内偏移量，所以这些位不能被占用。*/
  set_gatedesc(idt + 0x21, (int)asm_int_handler21, 2 * 8, AR_INTGATE32);
  set_gatedesc(idt + 0x27, (int)asm_int_handler27, 2 * 8, AR_INTGATE32);
  set_gatedesc(idt + 0x2c, (int)asm_int_handler2c, 2 * 8, AR_INTGATE32);
}

void set_segmdesc(struct SegmentDescriptor *sd, unsigned int limit, int base,
                  int ar) {
  if (limit > 0xfffff) {
    ar |= 0x8000; // G_bit = 1
    limit /= 0x1000;
  }

  sd->limit_low = limit & 0xffff;
  sd->base_low = base & 0xffff;
  sd->base_mid = (base >> 16) & 0xff;
  sd->access_right = ar & 0xff;
  sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) | 0xf0);
  sd->base_high = (base >> 24) & 0xff;
}

void set_gatedesc(struct GateDescriptor *gd, int offset, int selector, int ar) {
  gd->offset_low = offset & 0xffff;
  gd->selector = selector;
  gd->dw_count = (ar >> 8) & 0xff;
  gd->access_right = ar & 0xff;
  gd->offset_high = (offset >> 16) & 0xffff;
}
