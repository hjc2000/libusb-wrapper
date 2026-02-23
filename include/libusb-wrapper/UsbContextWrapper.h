#pragma once
#include <base/string/define.h>
#include <libusb-1.0/libusb.h>
#include <libusb-wrapper/UsbError.h>
#include <stdexcept>

namespace libusb
{
	class UsbContextWrapper
	{
	private:
		libusb_context *_wrapped_obj;

	public:
		/**
		 * @brief 初始化 USB 上下文。
		 * @exception Exception 初始化失败会抛出异常。
		 */
		UsbContextWrapper()
		{
			int result = libusb_init(&_wrapped_obj);
			if (result < 0)
			{
				throw std::runtime_error("初始化失败");
			}

			libusb_set_debug(_wrapped_obj, LIBUSB_LOG_LEVEL_INFO);
		}

		~UsbContextWrapper()
		{
			libusb_exit(_wrapped_obj);
			_wrapped_obj = nullptr;
		}

		libusb_context *WrappedObj() const
		{
			return _wrapped_obj;
		}
	};

} // namespace libusb
