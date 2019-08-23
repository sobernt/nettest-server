# nettest-server

server utility for testing network device.

usage:
nettest-server <nettest_ip> <comand>
    comands:
    ping,
    exit
for comand ping we has more arguments:
  usage: nettest-server <host> <command> <testing_ip> <type> <packetLength> <packetCount> <testTime>


comand "exit" do stop "nettest-client" program in remote device.
