# 定义生成规则
add_library("${ProjectName}" STATIC)
target_import_src(${ProjectName})
target_import_libusb(${ProjectName} PUBLIC)
target_import_base(${ProjectName} PUBLIC)
