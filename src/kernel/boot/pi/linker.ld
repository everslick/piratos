ENTRY(__reset)
SECTIONS
{
  . = 0x0;
  .text : {
    *(.reset)
    *(.text)
  }
 
  __exidx_start = .;
  .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
  __exidx_end = .;

  .data : { *(.data) }
  __bss_start__ = .;
  .bss : { *(.bss) }
  __bss_end__ = .;
}

