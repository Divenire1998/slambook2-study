#include "myslam/dataset.h"
#include "myslam/frame.h"

#include <boost/format.hpp>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "myslam/visual_odometry.h"
using namespace std;


namespace myslam{

    Dataset::Dataset(const std::string& dataset_path)
            : dataset_path_(dataset_path) {}


    bool Dataset::Init()
    {
        ifstream fin(dataset_path_ + "/calib.txt");
        if (!fin) {
            LOG(ERROR) << "cannot find " << dataset_path_ << "/calib.txt!";
            return false;
        }

        for (int i = 0; i < 4; ++i){   //一共有P0,P1,P2,P3四个相机，这里就是要把四个相机的参数全部读取到
            //前三个字符是P0：所以这里定义了一个长度为3的字符数组，读完这三个字符后就遇到了第一个空格，fin将会跳过这个空格，读取参数
            char camera_name[3];
            for (int k = 0; k < 3; ++k) {
                fin >> camera_name[k];
            }

            //将相机后面对应的12个参数读入到projection_data[12]中
            double projection_data[12];
            for (int k = 0; k < 12; ++k) {
                fin >> projection_data[k];
            }

            //将projection_data[12]的参数分为两部分提取出来
            Mat33 K;
            K << projection_data[0], projection_data[1], projection_data[2],
                    projection_data[4], projection_data[5], projection_data[6],
                    projection_data[8], projection_data[9], projection_data[10];

            Vec3 t;
            t << projection_data[3], projection_data[7], projection_data[11];

            // OPENCV的矩阵是列优先的，实际上就是计算相机之间的平移参数(米制) baseline
            t = K.inverse() * t;

            // 图像被resize了，那么对应的内参也要resize
            K = K * VisualOdometry::img_resize_;

			// 初始的旋转矩阵都是I，只有平移不一样
            Camera::Ptr new_camera(new Camera(K(0, 0), K(1, 1), K(0, 2), K(1, 2),
                                              t.norm(), SE3(SO3(), t)));
            cameras_.push_back(new_camera);
			
            LOG(INFO) << "Camera " << i << " extrinsics: " << t.transpose();

        }

        fin.close();

		//虽然类内已经对这个成员变量赋值了，但是这里还要写一个置0语句.
		// 每次初始化都要重新置0
        current_image_index_ = 0;
        return true;
    }

    Frame::Ptr Dataset::NextFrame()
    {

        boost::format fmt("%s/image_%d/%06d.png");

        //boost::format的相关内容和用法参考：
        ///https://blog.csdn.net/weixin_33802505/article/details/90594738?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromBaidu-1.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromBaidu-1.control


        cv::Mat image_left, image_right;
        
        // 灰度图形式读取
        image_left =
                cv::imread((fmt % dataset_path_ % 0 % current_image_index_).str(),
                           cv::IMREAD_GRAYSCALE);
        image_right =
                cv::imread((fmt % dataset_path_ % 1 % current_image_index_).str(),
                           cv::IMREAD_GRAYSCALE);

        if (image_left.data == nullptr || image_right.data == nullptr) {
            LOG(ERROR) << "cannot find images at index " << current_image_index_;
            return nullptr;
        }


        //利用resize()函数改变图像尺寸
        //resize(InputArray src, OutputArray dst, Size dsize,double fx=0, double fy=0, int interpolation=INTER_LINEAR )
        /*
         * InputArray src ：输入，原图像，即待改变大小的图像；
           OutputArray dst： 输出，改变后的图像。这个图像和原图像具有相同的内容，只是大小和原图像不一样而已；
           dsize：输出图像的大小。
           如果这个参数不为0，那么就代表将原图像缩放到这个Size(width，height)指定的大小；如果这个参数为0，那么原图像缩放之后的大小就要通过下面的公式来计算：
           dsize = Size(round(fxsrc.cols), round(fysrc.rows))

          其中，fx和fy就是下面要说的两个参数，是图像width方向和height方向的缩放比例。
          fx：width方向的缩放比例，如果它是0，那么它就会按照(double)dsize.width/src.cols来计算；
          fy：height方向的缩放比例，如果它是0，那么它就会按照(double)dsize.height/src.rows来计算；

          interpolation：这个是指定插值的方式，图像缩放之后，肯定像素要进行重新计算的，就靠这个参数来指定重新计算像素的方式，有以下几种：
          INTER_NEAREST - 最邻近插值
          INTER_LINEAR - 双线性插值，如果最后一个参数你不指定，默认使用这种方法
          INTER_AREA - resampling using pixel area relation. It may be a preferred method for image decimation, as it gives moire’-free results. But when the image is zoomed, it is similar to the INTER_NEAREST method.
          INTER_CUBIC - 4x4像素邻域内的双立方插值
          INTER_LANCZOS4 - 8x8像素邻域内的Lanczos插值
         */

        cv::Mat image_left_resized, image_right_resized;
        cv::resize(image_left, image_left_resized, cv::Size(), VisualOdometry::img_resize_, VisualOdometry::img_resize_,
                   cv::INTER_NEAREST);
		
        cv::resize(image_right, image_right_resized, cv::Size(), VisualOdometry::img_resize_, VisualOdometry::img_resize_,
                   cv::INTER_NEAREST);

		// 创建一个新的帧，自动分配ID号
        auto new_frame = Frame::CreateFrame();
		
		// 左右目
        new_frame->left_img_ = image_left_resized;
        new_frame->right_img_ = image_right_resized;
        current_image_index_++;

        return new_frame;

    }

}
