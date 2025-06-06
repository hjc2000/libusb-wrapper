#include <iostream>
#include <libusb-wrapper/UsbDeviceWrapper.h>

libusb::UsbDeviceWrapper::UsbDeviceWrapper(libusb_device *device)
{
	Ref(device);
}

libusb::UsbDeviceWrapper::UsbDeviceWrapper(UsbDeviceWrapper const &other)
{
	*this = other;
}

libusb::UsbDeviceWrapper::~UsbDeviceWrapper()
{
	Unref();
}

libusb::UsbDeviceWrapper &libusb::UsbDeviceWrapper::operator=(UsbDeviceWrapper const &other)
{
	Ref(other._wrapped_obj);
	_device_handle = other._device_handle;
	return *this;
}

/// <summary>
///		让 _wrapped_obj 指向一个设备并增加引用计数。
///		在引用任何设备前，会先自动调用 Unref 方法，这样如果 _wrapped_obj 原先有引用一个设备，会解除引用。
/// </summary>
/// <param name="device"></param>
void libusb::UsbDeviceWrapper::Ref(libusb_device *device)
{
	Unref();
	_wrapped_obj = device;
	libusb_ref_device(device);
}

/// <summary>
///		减少 _wrapped_obj 指向的设备的引用计数，然后将 _wrapped_obj 置为空。
/// </summary>
void libusb::UsbDeviceWrapper::Unref()
{
	libusb_unref_device(_wrapped_obj);
	_wrapped_obj = nullptr;
}

libusb_device_descriptor libusb::UsbDeviceWrapper::GetDescriptor()
{
	// 控制台输出时，输出 4 位，高位补 0 的方法：
	// cout << std::format("{:04x}", desc.idVendor) << endl;
	libusb_device_descriptor desc{};
	int ret = libusb_get_device_descriptor(_wrapped_obj, &desc);
	if (ret < 0)
	{
		throw std::runtime_error(UsbErrorCodeToString(ret));
	}

	return desc;
}

void libusb::UsbDeviceWrapper::Open()
{
	if (_device_handle)
	{
		std::cout << "设备已打开，不要重复打开" << std::endl;
		return;
	}

	libusb_device_handle *handle = nullptr;
	int ret = libusb_open(_wrapped_obj, &handle);
	if (ret)
	{
		throw std::runtime_error(UsbErrorCodeToString(ret));
	}

	_device_handle = shared_ptr<libusb_device_handle>{
		handle,
		[](libusb_device_handle *handle)
		{
			libusb_close(handle);
		},
	};
}

int libusb::UsbDeviceWrapper::ControlTransfer(USBRequestOptions request_type,
											  uint8_t request_cmd,
											  uint16_t value,
											  uint16_t index,
											  uint8_t *data,
											  uint16_t length,
											  uint32_t timeout)
{
	int result = libusb_control_transfer(_device_handle.get(),
										 request_type,
										 request_cmd,
										 value,
										 index,
										 data,
										 length,
										 timeout);

	return result;
}

uint16_t libusb::UsbDeviceWrapper::GetStatus()
{
	uint8_t status_buf[2];

	USBRequestOptions options{
		USBRequestOptions::DataDirection::DeviceToHost,
		USBRequestOptions::RequestType::Standard,
		USBRequestOptions::RecipientType::Device,
	};

	int have_read = ControlTransfer(options,
									libusb_standard_request::LIBUSB_REQUEST_GET_STATUS,
									0,
									0,
									status_buf,
									sizeof(status_buf),
									0);

	if (have_read != sizeof(status_buf))
	{
		throw std::runtime_error("获取状态信息失败");
	}

	return (status_buf[1] << 8) | status_buf[0];
}

int libusb::UsbDeviceWrapper::BulkTransfer(uint8_t endpoint, uint8_t *data, int length, uint32_t timeout)
{
	// 已传输的字节数
	int transferred = 0;
	int ret = libusb_bulk_transfer(_device_handle.get(),
								   endpoint,
								   data,
								   length,
								   &transferred,
								   timeout);

	if (ret < 0)
	{
		throw std::runtime_error(UsbErrorCodeToString(ret));
	}

	return transferred;
}

std::vector<shared_ptr<libusb::UsbConfigDescriptorWrapper>> libusb::UsbDeviceWrapper::GetConfigDescriptorList()
{
	std::vector<shared_ptr<UsbConfigDescriptorWrapper>> config_list;
	libusb_device_descriptor descriptor = GetDescriptor();
	for (uint8_t i = 0; i < descriptor.bNumConfigurations; i++)
	{
		libusb_config_descriptor *config = nullptr;
		int ret = libusb_get_config_descriptor(_wrapped_obj, i, &config);
		if (ret)
		{
			std::cout << CODE_POS_STR << UsbErrorCodeToString(ret) << std::endl;
			continue;
		}

		shared_ptr<UsbConfigDescriptorWrapper> wrapper{new UsbConfigDescriptorWrapper{config}};
		config_list.push_back(wrapper);
	}

	return config_list;
}

void libusb::UsbDeviceWrapper::ClaimInterface()
{
	std::vector<shared_ptr<UsbConfigDescriptorWrapper>> config_list = GetConfigDescriptorList();
	for (shared_ptr<UsbConfigDescriptorWrapper> &config : config_list)
	{
		ClaimInterface(*config);
	}
}

void libusb::UsbDeviceWrapper::ClaimInterface(UsbConfigDescriptorWrapper &config)
{
	for (uint8_t i = 0; i < config.InterfaceCount(); i++)
	{
		ClaimInterface(config.GetInterface(i));
	}
}

void libusb::UsbDeviceWrapper::ClaimInterface(libusb_interface const &interface)
{
	// 遍历接口中的所有设置（altsetting）
	for (int i = 0; i < interface.num_altsetting; i++)
	{
		libusb_interface_descriptor const *iface_desc = &interface.altsetting[i];
		int interface_number = iface_desc->bInterfaceNumber;
		ClaimInterface(interface_number);
	}
}

void libusb::UsbDeviceWrapper::ClaimInterface(int interface_number)
{
	int ret = libusb_claim_interface(_device_handle.get(), interface_number);
	if (ret)
	{
		std::cout << CODE_POS_STR << UsbErrorCodeToString(ret) << std::endl;
		throw std::runtime_error(UsbErrorCodeToString(ret));
	}
}
