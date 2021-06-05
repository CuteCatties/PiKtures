_Pragma("once");
#include<opencv2/opencv.hpp>
#include<vector>
#include<algorithm>
#include<concepts>
namespace PiKtures::Utility{
    enum class ErrorCode: unsigned int{
        OK = 0,
        COMMAND_NOT_FOUND,
        COMMAND_AMBIGUOUS,
        PARAMETER_TOO_LESS,
        PARAMETER_TOO_MUCH,
        EVEN_KERNEL_SIZE,
        ODD_KERNEL_SIZE,
        KERNEL_TOO_BIG,
        KERNEL_TOO_SMALL,
        INVALID_IMAGE_TYPE,
        PROBABILITY_OUT_OF_RANGE,
        PROBABILITIES_OUT_OF_RANGE,
        IMAGE_TOO_SMALL,
        NONPOSITIVE_DIAMETER,
        EMPTY_COMMAND,
        NEGATIVE_DEVIATION,
        WATERMARK_TOO_BIG,
        MODULE_LOAD_FAILED,
        NO_ACTIVE_MODULE,
        NO_IMAGE_OPENED,
        // !!! insert new error codes *RIGHT ABOVE* this comment !!!
        // don't forget to insert a new error message line in utility.cpp
        // you won't forget it, otherwise the static assert would fail.
        ERRORCODERARE
    };
    const char* errorMessage(const unsigned int);
    inline const char* errorMessage(const ErrorCode& ec){return errorMessage(static_cast<unsigned int>(ec));}
    using cv::Mat;
    Mat alignImage(const Mat&);
    inline Mat alignImage(const Mat& origin){
        return origin(cv::Rect(0, 0, origin.cols & -2, origin.rows & -2));
    }
    template<std::floating_point T>
    inline bool checkProbability(const std::vector<T>& probabilities){
        T sum = 0.0;
        for(const auto& x: probabilities){
            if(x > 1.0 || x < 0.0) throw ErrorCode::PROBABILITY_OUT_OF_RANGE;
            if((sum += x) > 1.0) throw ErrorCode::PROBABILITIES_OUT_OF_RANGE;
        }
        return false;
    }
    inline bool checkImage(Mat& image){
        if(image.cols < 2 || image.rows < 2) throw ErrorCode::IMAGE_TOO_SMALL;
        if(image.channels() == 4){   // assume that picture with 4 channels HAS RGBA-color
            // cruelly elimate the alpha channel
            cv::cvtColor(image, image, cv::COLOR_RGBA2RGB);
        }else if(image.channels() != 3) throw ErrorCode::INVALID_IMAGE_TYPE;
        // Of course, we assume that picture with 3 channels HAS RGB-color.
        // NO EXCEPTION
        return false;
    }
    Mat calculateSpectrum(const Mat&, int);
    Mat transformSpectrum(const Mat&);
    Mat visualizeSpectrum(const Mat&);
    template<typename...Args, std::invocable<Mat&, Args...> F>
    void applyToEachChannel(F&& function, Mat& result, const Mat& m, Args&&...args){
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