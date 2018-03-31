binary="bin/sar.so"

exe="portal2_linux"
appid="620"

#exe="hl2_linux"
#appid="400"

pid=$(pidof $exe)

if [ -z "$pid" ]; then
  echo -e "\e[31mProcess not found!\e[0m"
  echo -e "\e[34mWanna start it (y/n)? \e[0m"
  read ye
  if [ "$ye" == "y" ]; then
  	steam steam://run/"$appid"
  fi
  exit
fi

if [ grep -q $binary /proc/$pid/maps ]; then
  echo -e "\e[34mSAR already injected!\e[0m"
  exit
fi

load="$(
sudo gdb -n -q -batch \
  -ex "attach $pid" \
  -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
  -ex "call \$dlopen(\"$(pwd)/$binary\", 1)" \
  -ex "detach" \
  -ex "quit"
)"

result="${load##*$'\n'}"

if [ "$result" != "\$1 = (void *) 0x0" ]; then
  echo -e "\e[32mInjected $binary into $exe!\e[0m"
else
  echo -e "\e[31mInjection failed!\e[0m"
fi