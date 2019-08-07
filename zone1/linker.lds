/* Copyright(C) 2018 Hex Five Security, Inc. - All Rights Reserved */

OUTPUT_ARCH( "riscv" )

ENTRY( _start )

INCLUDE memory.lds

SECTIONS {
  __stack_size = DEFINED(__stack_size) ? __stack_size : 2K;

  .init           : ALIGN(8)  {
    KEEP (*(SORT_NONE(.init)))
  } >ram_code AT>flash 
  
  .text   :  ALIGN(8) { 
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text .text.*)
    *(.gnu.linkonce.t.*) 
  } >ram_code AT>flash 

  .fini           :  ALIGN(8) {
    KEEP (*(SORT_NONE(.fini)))
  } >ram_code AT>flash

  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);

  .rodata         :   ALIGN(8) {
    *(.rdata)
    *(.rodata .rodata.*)
    *(.gnu.linkonce.r.*)
  } >ram_code AT>flash

  

  .preinit_array  :  ALIGN(8)  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >ram_code AT>flash

  .init_array     :  ALIGN(8)  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >ram_code AT>flash

  .fini_array     : ALIGN(8)   {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >ram_code AT>flash  

  .ctors          : ALIGN(8)   {
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  } >ram_code AT>flash 

  .dtors          : ALIGN(8)   {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  } >ram_code AT>flash 

  .lalign         :   ALIGN(8) {
    PROVIDE( _data_lma = . );
  } >ram_code AT>flash  

  .dalign         :   ALIGN(8) {
    PROVIDE( _data = . );
  } >ram_data AT>flash 

  .data          :  ALIGN(8) {
    *(.data .data.*)
    *(.gnu.linkonce.d.*)
    . = ALIGN(8);
    PROVIDE( __global_pointer$ = . + 0x800 );
    *(.sdata* .sdata.*)
    *(.gnu.linkonce.s.*)
    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)
  } >ram_data AT>flash

  . = ALIGN(8);
  PROVIDE( _edata = . );
  PROVIDE( edata = . );

  PROVIDE( _fbss = . );
  PROVIDE( __bss_start = . );
  .bss            :   {
    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(8);
  } >ram_data AT>ram_data 

  . = ALIGN(8);
  PROVIDE( _end = . );
  PROVIDE( end = . );

  .stack ORIGIN(ram_data) + LENGTH(ram_data) - __stack_size :   {
    PROVIDE( _heap_end = . );
    . = __stack_size;
    PROVIDE( _sp = . );
  } >ram_data AT>ram_data 

}