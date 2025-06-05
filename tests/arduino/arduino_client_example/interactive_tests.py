import minimalmodbus
import serial
import os
import time


def main():
    print("starting interactive modbus client tests")
    port = os.environ.get("SERIAL_PORT", "/dev/ttyNONE_SPECIFIED")
    print("using serial port:", port)
    inst = minimalmodbus.Instrument(port, 2)  # port name, slave address (in decimal)
    # inst.debug = True  # enable debugging
    inst.serial.baudrate = 115200
    inst.serial.timeout = 1
    time.sleep(1) # Wait for the serial port to be ready
    # Prime the pump, ignore first error if COMM is in a weird state
    try:
        inst.read_registers(0, 1)  # Read a register to ensure the connection is established
    except Exception as e:
        print("Ignoring first error")

    print("read register tests:", end=' ')
    # here we read 100 registers starting at 0x00
    inst.read_registers(0, 1)  # read out 1 registers starting at address 0x00
    print ("PASSED")
    # Now read from the max register location
    data = inst.read_registers(0x1234, 1)  # Read the maximum register
    print("read max register location:", hex(0x1234), "data:", hex(data[0]), "expected:", hex(0xABCD))
    assert(data[0] == 0xABCD)
    data = inst.read_registers(0x5678, 1)
    assert(data[0] == 0xEF01)
    print("PASSED")
    print("write register tests:", end=' ')
    # Now write 10 registers starting at 0x00
    length = 10
    inst.write_registers(0, [i for i in range(length)])  # start address, list of values
    # Now write to the max register location
    read_back = inst.read_registers(0, length)  # Read out 100 registers starting at address 0x00
    for i in range(length):
        assert(read_back[i] == i)
    inst.write_registers(0, [0]*length)  # Write to the maximum register
    read_back = inst.read_registers(0, length)  # Read out 100 registers starting at address 0x00
    for i in range(length):
        assert(read_back[i] == 0)
    print("PASSED")
    print("")
    print("all tests completed successfully")

if __name__ == "__main__":
    main()