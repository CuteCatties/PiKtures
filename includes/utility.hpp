_Pragma("once");
#include<opencv2/opencv.hpp>
#include<vector>
#include<algorithm>
#include<concepts>
#include<iostream>
using namespace std;
namespace PiKtures::Utility{
    enum class ErrorCode: unsigned int{
        OK = 0,
        NO_MODIFYCATION,
        NO_STACK_MODIFICATION,
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
        EMPTY_PAST_STACK,
        EMPTY_FUTURE_STACK,
        FILE_UNACCESSIABLE,
        FILESYSTEM_ERROR,
        // !!! insert new error codes *RIGHT ABOVE* this comment !!!
        // don't forget to insert a new error message line in utility.cpp
        // you won't forget it, otherwise the static assert would fail.
        ERRORCODERARE
    };
    const char* errorMessage(const unsigned int);
    inline const char* errorMessage(const ErrorCode& ec){return errorMessage(static_cast<unsigned int>(ec));}
    inline bool isError(const ErrorCode& ec){
        return ec != ErrorCode::OK &&
               ec != ErrorCode::NO_MODIFYCATION &&
               ec != ErrorCode::NO_STACK_MODIFICATION;
    }
    inline bool isModified(const ErrorCode& ec){
        return ec == ErrorCode::OK;
    }
    inline bool maintainStacks(const ErrorCode& ec){
        return ec != ErrorCode::NO_STACK_MODIFICATION;
    }
    inline bool isError(const unsigned int ec){
        return isError(static_cast<ErrorCode>(ec));
    }
    inline bool isModified(const unsigned int ec){
        return isModified(static_cast<ErrorCode>(ec));
    }
    inline bool maintainStacks(const unsigned int ec){
        return maintainStacks(static_cast<ErrorCode>(ec));
    }
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
    inline Mat getBoundarySelector(const Mat& selector, int width){
        int x_start = 0, x_end = 0, y_start = 0, y_end = 0;
        for(int x = 0; x < selector.rows; ++x) for(int y = 0; y < selector.cols; ++y) if(selector.at<uint8_t>(x, y) == 1){
            x_start = x;
            y_start = y;
            goto jo;
        }
        jo:
        for(int y = y_start; selector.at<uint8_t>(x_start, y) == 1; y_end = ++y);
        for(int x = x_start; selector.at<uint8_t>(x, y_start) == 1; x_end = ++x);
        Mat result = selector.clone();
        if(x_end <= x_start || y_end <= y_start){
            result *= 0;
        }else{
            using std::min;
            using std::max;
            Mat outter = result(
                cv::Range(max(0, x_start - width), min(selector.rows, x_end + width)),
                cv::Range(max(0, y_start - width), min(selector.cols, y_end + width))
            );
            outter *= 0;
            outter += 1;
            if((x_start + (width << 1) < x_end) && (y_start + (width << 1) < y_end)){
                Mat inner = result(
                    cv::Range(x_start + width, x_end - width),
                    cv::Range(y_start + width, y_end - width)
                );
                inner *= 0;
            }
        }
        return result;
    }
    inline Mat reverseSelector(const Mat& selector){
        Mat result(selector.size(), selector.type());
        selector.forEach<uint8_t>([&result](const uint8_t& value, const int position[])->void{
            result.at<uint8_t>(position) = value ? 0 : 1;
        });
        return result;
    }
}