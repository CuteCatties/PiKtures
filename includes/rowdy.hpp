#include<opencv2/opencv.hpp>
namespace PiKtures::Rowdy{
    using cv::Mat;
    void salt(Mat&, double, double);
    void random(Mat&, double);
    void gaussian(Mat&, double, double);
    void blindWatermark(Mat&, const char* const, double);
    void uncoverWatermark(Mat&);
}