_Pragma("once");
#include<utility.hpp>
#include<opencv2/opencv.hpp>
#include<cmath>
#include<functional>
#include<concepts>
#include<algorithm>
namespace PiKtures::Enhancer{
    using cv::Mat;
    using PiKtures::Utility::ErrorCode;
    inline double gaussian(int x, int y, double sigma){
        sigma *= sigma;
        return std::exp((-x * x - y * y) / sigma * 0.5) / M_PI / sigma * 0.5;
    }
     inline int distance2(int x, int x0, int y, int y0){return (x - x0) * (x - x0) + (y - y0) * (y - y0);}
    inline double distance(int x, int x0, int y, int y0){return std::sqrt(distance2(x, x0, y, y0));}
    inline double butterworthLowpass(int x, int x0, int y, int y0, double d, double n){
        return 1 / (1 + std::pow(distance(x, x0, y, y0) / d, 2 * n));
    }
    inline double butterworthHighpass(int x, int x0, int y, int y0, double d, double n){
        return 1 / (1 + std::pow(d / distance(x, x0, y, y0), 2 * n));
    }
    inline double gaussianLowpass(int x, int x0, int y, int y0, double d){
        return std::exp(-distance2(x, x0, y, y0) / (d * d) * 0.5);
    }
    inline double gaussianHighpass(int x, int x0, int y, int y0, double d){
        return 1 - gaussianLowpass(x, x0, y, y0, d);
    }
    template<typename...Args, std::invocable<int, int, int, int, Args...> F>
    std::enable_if<std::is_convertible<std::invoke_result_t<F, int, int, int, int, Args...>, double>::value, cv::Mat>::type
    getPassKernel(const Mat& image, F&& f, Args&&...args){
        if(image.cols == 1 || image.rows == 1) throw ErrorCode::IMAGE_TOO_SMALL;
        Mat kernel(image.size(), CV_32F);
        cv::Point center(kernel.rows >> 1, kernel.cols >> 1);
        kernel.forEach<float>([&center, &args..., &f](float& pixel, const int* position){
            pixel = f(position[0], center.x, position[1], center.y, std::forward<Args>(args)...);
        });
        return kernel;
    }
    Mat getGaussianKernel(int, double);
    void applyPassKernel(Mat&, const Mat&);
    void medianFilter(Mat&, int);
    void meanFilter(Mat&, int);
    void gaussianFilter(Mat&, int, double);
    void bilateralFilter(Mat&, double, double);
    void butterworthLowpassFilter(Mat&, double, double);
    void butterworthHighpassFilter(Mat&, double, double);
    void gaussianLowpassFilter(Mat&, double);
    void gaussianHighpassFilter(Mat&, double);
    void dermabrasion(Mat&, double, double);
    void luminance(Mat&, double, int);
}