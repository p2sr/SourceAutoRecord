binary="bin/sar.so"
pid=$(pidof portal2_linux)

if [ -z "$pid" ]; then
  echo -e "\e[31mPortal 2 process not found!\e[0m"
  exit
fi

"$(
sudo gdb -n -q -batch \
  -ex "attach $pid" \
  -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
  -ex "set \$dlclose = (int(*)(void*)) dlclose" \
  -ex "set \$library = \$dlopen(\"$(pwd)/$binary\", 6)" \
  -ex "call \$dlclose(\$library)" \
  -ex "call \$dlclose(\$library)" \
  -ex "detach" \
  -ex "quit"
)"