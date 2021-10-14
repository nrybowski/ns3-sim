# zebra 8.0.1
## isis_test: caps_to_text segfault
- Not in old quagga
==12779==    at 0x82B67E0: strchrnul (strchr.S:24)
==12779==    by 0x826D217: __find_specmb (printf-parse.h:108)
==12779==    by 0x826D217: vfprintf (vfprintf.c:1312)
==12779==    by 0x8296A58: vsnprintf (vsnprintf.c:114)
==12779==    by 0x5003837: dce_vsnprintf (dce-stdio.cc:832)
==12779==    by 0xC1BA4B6: __sprintf_chk (libc.cc:156)
==12779==    by 0xCD59B97: cap_to_text (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libcap.so.2)
==12779==    by 0xD9D35BF: zprivs_caps_init (privs.c:325)
==12779==    by 0xD9D411E: zprivs_init (privs.c:673)
==12779==    by 0xD9A0C11: frr_init (libfrr.c:749)
==12779==    by 0xDEB1F74: main (main.c:390)

## isis_test: libyang segfault 
==30940==    at 0x82B67E0: strchrnul (strchr.S:24)
==30940==    by 0x826D217: __find_specmb (printf-parse.h:108)
==30940==    by 0x826D217: vfprintf (vfprintf.c:1312)
==30940==    by 0x8296782: vasprintf (vasprintf.c:59)
==30940==    by 0x5003760: dce_vasprintf (dce-stdio.cc:807)
==30940==    by 0x5003461: dce_asprintf (dce-stdio.cc:790)
==30940==    by 0xC18A642: asprintf (libc-ns3.h:345)
==30940==    by 0xD6BEA2C: lyplg_type_store_uint (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==30940==    by 0xD64FFDF: lys_compile_unres_dflt.isra.3 (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==30940==    by 0xD6530D0: lys_compile_unres_glob (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==30940==    by 0xD652E42: lys_recompile (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==30940==    by 0xD653054: lys_compile_unres_glob (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==30940==    by 0xD6315E2: ly_ctx_load_module (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2

==24576==    at 0x4C2FFC6: memalign (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==24576==    by 0x4013037: allocate_and_init (dl-tls.c:603)
==24576==    by 0x4013037: tls_get_addr_tail (dl-tls.c:791)
==24576==    by 0xD62726F: ly_log_location (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==24576==    by 0xD682AD1: yang_parse_module (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==24576==    by 0xD668644: lys_create_module (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==24576==    by 0xD632C8C: ly_ctx_new (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libyang.so.2)
==24576==    by 0xDA14468: yang_ctx_new_setup (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libfrr.so.0)
==24576==    by 0xDA144EE: yang_init (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libfrr.so.0)
==24576==    by 0xD9C06EF: nb_init (in /home/ns3dce/dce-linux-dev/source/ns-3-dce/elf-cache/0/libfrr.so.0)
==24576==    by 0xD9A0C92: frr_init (libfrr.c:777)
==24576==    by 0xDEB1F74: main (main.c:390)

Myscript:
- Made setgroups error non fatal (Bad file descriptor: seems to be wrong errno => same error while plain zebra without root)
- removed cap_to_text (segfault on vsnprintf)
- Segfault on vasprintf (native and DCE wrapper)
  - in libyang parser

Quagga script:
- v0.99.20 (29/09/2011)
- Invalid read in zprivs_init => Segfault (stack allocation)
