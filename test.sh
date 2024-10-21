#!/bin/bash
meson compile -C build
meson test -C build -v

: '
gdb build/storage_cow_test
(gdb) run
(gdb) bt
'