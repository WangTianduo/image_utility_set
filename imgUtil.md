libimg0626.a静态库文档
===========


<br />

##1. 头文件 imgStruct.h
-----------
+ namespace：imgUtil
+ 输入图片 struct: ManagedImage

 - 一共四个量：
     -  1. (int)ManagedImage.height
     -  2. (int)ManagedImage.width
     -  3. (size_t)ManagedImage.data.size
     -  4. (unique_ptr<uint8_t>)ManagedImage.data,buffer.get()


+ 输出图片struct: OutputImage
   -   一共四个量：
      -  1. (int)OutputImage.height
      -  2. (int)OutputImage.width 
       -  3. (size_t)OutputImage.data.size
       -  4. (unsigned char*)OutputImage.data.buffer

<br data-effect="nomal"/>

##2. 头文件 jpeg_core.h
------------
+ namespace:  imgUtil
+ 功能：
 - 读取jpeg文件为rgb, yuvnv12(ios), yuvnv21(Android)数据buffer；
 - 将rgb，yuvnv12, yunvnv21 输出为jpeg格式的文件
+ int rgb2jpeg(OutputImage &out_image, const char *jpeg_file);
+ int yunvnv122jpeg(OutputImage &out_image, const char *jpeg_file);
+ int yuvnv212jpeg(OutputImage &out_image, const char *jpeg_file);
+ int jpeg2rgb(ManagedImage &image, const char *jpeg_file);
+ int jpeg2yuvnv12(ManagedImage &image, const char *jpeg_file);
+ int jpeg2yuvnv21(ManagedImage &image, const char *jpeg_file);


<br data-effect="slide"/>

##3. 头文件 format_change.h
---------------------
+ namespace: imgUtilTools
+ 功能：
  - 图片格式相互转换(rgb, bgr, rgba, yuvnv21, yuvnv12,)共20个
  -  图片旋转(rgb, rgba, yuv420p)
  - 图片裁剪
+ 图片格式转换
 - void bgr2rgb(const unsigned char* src, unsigned char* dst, int img_width, int height);
 -  void rgb2yuvnv21(const unsigned char* src, unsigned char* dst, int img_width, int height);
 - void yuvnv212rgb(const unsigned char* src, unsigned char* dst, int img_width, int height);
 - 剩余17个函数命名法相同， 参数列表相同
+ 图片旋转
 -  void rotate_rgb(unsigned char* src, int src\_width, int src\_height, int orientation, unsigned char* dst, int& dst\_width, int& dst\_height);
 - void rotate_yuv420p(unsigned char* src, int src\_width, int src\_height, int orientation, unsigned char* dst, int& dst\_width, int& dst\_height);
+ 图片裁剪
 - void resize\_specific\_size(unsigned char* src, MGHUM_TRANSFORM_IMAGE_TYPE img_type, int src\_width, int src\_height, int dst\_width, int dst\_height, unsigned char* dst);
 - void resize\_scale(const unsigned char* src, MGHUM_TRANSFORE_IMAGE_TYPE image\_type, int src\_width, int src\_height, int scale, unsigned char* dst, int& dst\_width, int& dst\_height);


<br data-effect="turn"/>
