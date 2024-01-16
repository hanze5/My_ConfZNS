#创建
sudo ip tuntap add dev tap0 mode tap
#启用
sudo ip link set tap0 up
#配置
sudo ip addr add 192.168.12.191/24 dev tap0

#创建桥接器
sudo ip link add name br0 type bridge
sudo ip link set br0 up

#桥接tap设备
sudo ip link set tap0 master br0

sudo ip tuntap del dev tap0 mode tap