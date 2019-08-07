/* Copyright(C) 2018 Hex Five Security, Inc. - All Rights Reserved */

MEMORY {
  flash    (rxai!w) : ORIGIN = 0x20040000, LENGTH = 28K  /* SPI0 FLASH 0x2000_0000-0x2080_0000   8M */
  ram_code (rxai!w) : ORIGIN = 0x08018000, LENGTH = 28K  /* L2LIM      0x0800_0000-0x0802_0000 128K */
  ram_data (wxa!ri) : ORIGIN = 0x0801F000, LENGTH = 4K   /* L2LIM      0x0800_0000-0x0802_0000 128K */
}

