_Pragma("once");
#include<opencv2/opencv.hpp>
#include<vector>
#include<algorithm>
namespace PiKtures::Utility{
    using cv::Mat;
    Mat alignImage(const Mat&);
    inline Mat alignImage(const Mat& origin){
        return origin(cv::Rect(0, 0, origin.cols & -2, origin.rows & -2));
    }
    template<typename T>
    inline bool checkPossibility(const std::vector<T>& possibilities){
        T sum = 0.0;
        for(const auto& x: possibilities){
            if(x >= 1.0 || x <= 0.0) return true;
            if((sum += x) >= 1.0) return true;
        }
        return false;
    }
    inline bool checkImage(Mat& image){
        if(image.channels() == 4){   // assume that picture with 4 channels HAS RGBA-color
            // cruelly elimate the alpha channel
            cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
        }else if(image.channels() != 3) return true;
        // Of course, we assume that picture with 3 channels HAS RGB-color.
        // NO EXCEPTION
        return false;
    }
    Mat transformSpectrum(const Mat&);
    Mat visualizeSpectrum(const Mat&);
    template<typename F, typename...Args>
    void applyToEachChannel(F function, Mat& result, const Mat& m, Args&&...args){
        std::vector<Mat> buffer;
        buffer.resize(m.channels());
        cv::split(m, buffer.data());
        for(auto& c: buffer) function(c, std::forward<Args>(args)...);
        cv::merge(buffer.data(), buffer.size(), result);
    }
    inline Mat reverseSelector(const Mat& selector){
        Mat result(selector.size(), selector.type());
        selector.forEach<uint8_t>([&result](const uint8_t& value, const int position[])->void{
            result.at<uint8_t>(position) = value ? 0 : 1;
        });
        return result;
    }
}