binary="bin/sar.so"

if grep -q $binary /proc/$(pidof portal2_linux)/maps; then
  sudo gdb -n -q -batch \
    -ex "attach $(pidof portal2_linux)" \
    -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
    -ex "set \$dlclose = (int(*)(void*)) dlclose" \
    -ex "set \$library = \$dlopen(\"$(pwd)/$binary\", 6)" \ # RTLD_NOW | RTLD_NOLOAD
    -ex "call \$dlclose(\$library)" \
    -ex "call \$dlclose(\$library)" \
    -ex "detach" \
    -ex "quit"
fi