<<<<<<< HEAD
add_library("${ProjectName}")
=======
# 定义生成规则
add_library("${ProjectName}" STATIC)
>>>>>>> 0db458b5efe86590463eec519cbdac05ef0f50c8
target_import_src(${ProjectName})
target_import_libusb(${ProjectName} PUBLIC)
target_import_base(${ProjectName} PUBLIC)
