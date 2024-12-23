#include<format>
#include<iostream>
#include<libusb-1.0/libusb.h>
#include<libusb-wrapper/UsbContextWrapper.h>
#include<libusb-wrapper/UsbDeviceListWrapper.h>
#include<libusb-wrapper/UsbDeviceWrapper.h>
#include<test_libusb.h>

using namespace libusb;

void test_libusb()
{
	try
	{
		UsbContextWrapper usb_ctx { };
		UsbDeviceListWrapper device_list;
		device_list.FindDevices(usb_ctx);
		std::cout << "找到 " << device_list.Count() << " 个设备" << std::endl;
		for (UsbDeviceWrapper device : device_list)
		{
			try
			{
				device.Open();
			}
			catch (...)
			{
				continue;
			}

			libusb_device_descriptor desc = device.GetDescriptor();
			std::cout << std::format("{:x}  {:x}", desc.idVendor, desc.idProduct) << std::endl;
		}
	}
	catch (std::runtime_error &ex)
	{
		std::cout << ex.what() << std::endl;
	}
}