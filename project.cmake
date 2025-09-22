add_library("${ProjectName}")
target_import_src(${ProjectName})
target_import_libusb(${ProjectName} PUBLIC)
target_import_base(${ProjectName} PUBLIC)
