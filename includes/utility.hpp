_Pragma("once");
#include<opencv2/opencv.hpp>
namespace PiKtures::Utility{
    using cv::Mat;
    Mat alignImage(const Mat&);
    inline Mat alignImage(const Mat& origin){
        return origin(cv::Rect(0, 0, origin.cols & -2, origin.rows & -2));
    }
    Mat transformSpectrum(const Mat&);
    Mat visualizeSpectrum(const Mat&);
}