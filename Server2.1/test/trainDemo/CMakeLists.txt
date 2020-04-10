# CMakeLists.txt

set(TEST_TARGET train-Test)
# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(. DIR_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/trainTemplate/ XXHASH_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/pc_dsplib/ DSPLIB_SRCS)

include_directories(
	${PROJECT_SOURCE_DIR}/trainTemplate/
	${PROJECT_SOURCE_DIR}/pc_dsplib/
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

# 指定生成 执行文件
add_executable(${TEST_TARGET} ${DIR_SRCS} ${XXHASH_SRCS} ${DSPLIB_SRCS})

# link library
target_link_libraries(${TEST_TARGET} -lz -ldl -lm -lpthread -lrt)
target_link_libraries(${TEST_TARGET} -lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core)
target_link_libraries(${TEST_TARGET} -littnotify -llibprotobuf -llibjpeg -llibwebp -llibpng -llibtiff -llibjasper -lIlmImf -lippiw -lippicv)

INSTALL(TARGETS ${TEST_TARGET} 
	RUNTIME DESTINATION bin)
