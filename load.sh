binary="bin/sar.so"
pid=$(pidof portal2_linux)

if [ -z "$pid" ]; then
  echo -e "\e[31mPortal 2 process not found!\e[0m"
  exit
fi

if [ grep -q $binary /proc/$pid/maps ]; then
  echo -e "\e[34mSAR already injected!\e[0m"
  exit
fi

load = "$(
sudo gdb -n -q -batch \
  -ex "attach $pid" \
  -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
  -ex "call \$dlopen(\"$(pwd)/$binary\", 1)" \
  -ex "detach" \
  -ex "quit"
)"

result="${load##*$'\n'}"

if [ "$result" != "\$1 = (void *) 0x0" ]; then
  echo -e "\e[32mInjected $binary into portal2_linux!\e[0m"
else
  echo -e "\e[31mInjection failed!\e[0m"
fi