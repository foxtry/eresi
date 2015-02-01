#!/bin/bash

#./configure --enable-32 --enable-readline --libasm-ia32 --libasm-arm --libasm-packed --enable-testing --use-ctags --set-lib-path "/lib;/lib/i386-linux-gnu;/usr/lib/i386-linux-gnu;/usr/lib;/usr/local/lib"
./configure --enable-32-64 --enable-readline --libasm-ia32 --libasm-packed --enable-testing --use-ctags --set-lib-path "/lib;/lib/i386-linux-gnu;/usr/lib/i386-linux-gnu;/usr/lib;/usr/local/lib"
