#include<enhancer.hpp>
#include<utility.hpp>
#include<opencv2/opencv.hpp>
#include<vector>
#include<algorithm>
using PiKtures::Utility::ErrorCode;
void PiKtures::Enhancer::applyLowPassKernel(Mat& image, const Mat& kernel){
    if(image.cols == 1 || image.rows == 1) throw ErrorCode::IMAGE_TOO_SMALL;
    if(image.cols != kernel.cols || image.rows != kernel.rows) throw std::logic_error("panic: Size of image and kernel does not match.");
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    PiKtures::Utility::applyToEachChannel(
        [&kernel](Mat& c){
            Mat buffer[2];
            c.convertTo(buffer[0], CV_32F);
            buffer[1] = Mat::zeros(buffer[0].size(), buffer[0].type());
            Mat operational_matrex;
            cv::merge(buffer, 2, operational_matrex);
            cv::dft(operational_matrex, operational_matrex);
            cv::split(operational_matrex, buffer);
            PiKtures::Utility::transformSpectrum(buffer[0]);
            PiKtures::Utility::transformSpectrum(buffer[1]);
            cv::multiply(buffer[0], kernel, buffer[0]);
            cv::multiply(buffer[1], kernel, buffer[1]);
            PiKtures::Utility::transformSpectrum(buffer[0]);
            PiKtures::Utility::transformSpectrum(buffer[1]);
            cv::merge(buffer, 2, operational_matrex);
            cv::idft(operational_matrex, operational_matrex, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
            operational_matrex.convertTo(c, CV_8U);
        }, image, image
    );
}
void PiKtures::Enhancer::medianFilter(cv::Mat& image, int kernel_size){
    if((kernel_size & 1) == 0) throw ErrorCode::EVEN_KERNEL_SIZE;
    if(kernel_size == 1) return;
    if(kernel_size > image.rows || kernel_size > image.cols) throw ErrorCode::KERNEL_TOO_BIG;
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    int offset = kernel_size >> 1;
    PiKtures::Utility::applyToEachChannel(
        [offset](Mat& c){
            Mat operational_matrix(c, cv::Range(offset, c.rows - offset + 1), cv::Range(offset, c.cols - offset + 1));
            Mat origin = c.clone();
            operational_matrix.forEach<uint8_t>([offset, &origin](uint8_t& pixel, const int* position){
                std::vector<uint8_t> kernel_items;
                for(int i = -offset; i <= offset; ++i) for(int j = -offset; j <= offset; ++j){
                    kernel_items.push_back(origin.at<uint8_t>(position[0] + i, position[1] + j));
                }
                std::nth_element(kernel_items.begin(), kernel_items.begin() + (kernel_items.size() >> 1), kernel_items.end());
                pixel = kernel_items[kernel_items.size() >> 1];
            });
        },
        image, image
    );
}
void PiKtures::Enhancer::meanFilter(cv::Mat& image, int kernel_size){
    if((kernel_size & 1) == 0) throw ErrorCode::EVEN_KERNEL_SIZE;
    if(kernel_size == 1) return;
    if(kernel_size > image.rows || kernel_size > image.cols) throw ErrorCode::KERNEL_TOO_BIG;
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    int offset = kernel_size >> 1;
    int kernel_elements = kernel_size * kernel_size;
    PiKtures::Utility::applyToEachChannel(
        [offset, kernel_elements](Mat& c){
            Mat operational_matrix(c, cv::Range(offset, c.rows - offset), cv::Range(offset, c.cols - offset));
            Mat origin = c.clone();
            for(int i = 0; i < operational_matrix.cols; ++i)
                for(int j = 0; j < operational_matrix.rows; ++j){
                    int sum = 0;
                    for(int k = 0; k < offset * 2 + 1; ++k){
                        for(int l = 0; l < offset * 2 + 1; ++l){
                            sum += origin.at<uint8_t>(j + l, i + k);
                        }
                    }
                    operational_matrix.at<uint8_t>(j, i) = cv::saturate_cast<uint8_t>(sum / kernel_elements);
                }
        },
        image, image
    );
}
void PiKtures::Enhancer::gaussianFilter(Mat& image, int kernel_size, double sigma){
    if((kernel_size & 1) == 0) throw ErrorCode::EVEN_KERNEL_SIZE;
    if(kernel_size == 1) return;
    if(kernel_size > image.rows || kernel_size > image.cols) throw ErrorCode::KERNEL_TOO_BIG;
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    int offset = kernel_size >> 1;

    // generate gaussian kernel
    Mat gaussian_kernel(cv::Size(kernel_size, kernel_size), CV_32F);
    // fill directly from gaussian function
    gaussian_kernel.forEach<float>([offset, sigma](float& p, const int* position){
        p = static_cast<double>(gaussian(position[0] - offset, position[1] - offset, sigma));
    });
    // normalize kernel
    gaussian_kernel /= cv::sum(gaussian_kernel)[0];
    // gaussian kernel is ready
    PiKtures::Utility::applyToEachChannel(
        [offset, &gaussian_kernel](Mat& c){cv::filter2D(c, c, -1, gaussian_kernel);},
        image, image
    );
}
void PiKtures::Enhancer::bilateralFilter(Mat& image, double color_sigma, double space_sigma){
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    Mat result(image.size(), image.type());
    cv::bilateralFilter(image, result, -1, color_sigma, space_sigma);
    image = result;
}
void PiKtures::Enhancer::butterworthLowpassFilter(Mat& image, double d, double n){
    if(d <= 0) throw ErrorCode::NONPOSITIVE_DIAMETER;
    if(image.cols == 1 || image.rows == 1) throw ErrorCode::IMAGE_TOO_SMALL;
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    Mat kernel = getPassKernel(image, butterworthLowpass, d, n);
    applyLowPassKernel(image, kernel);
}
void PiKtures::Enhancer::gaussianLowpassFilter(Mat& image, double d){
    if(d <= 0) throw ErrorCode::NONPOSITIVE_DIAMETER;
    if(image.cols == 1 || image.rows == 1) throw ErrorCode::IMAGE_TOO_SMALL;
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    Mat kernel = getPassKernel(image, gaussianLowpass, d);
    applyLowPassKernel(image, kernel);
}
void PiKtures::Enhancer::dermabrasion(Mat& image, double color_sigma, double space_sigma){
    // the kernel is never modified, define as const static to reduce the cost of time and space
    const static Mat kernel = (cv::Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    bilateralFilter(image, color_sigma, space_sigma);
    filter2D(image, image, -1, kernel, cv::Point(-1,-1), 0);
}
void PiKtures::Enhancer::luminance(cv::Mat& image, double rate, int bonus){
    if(PiKtures::Utility::checkImage(image)) throw ErrorCode::INVALID_IMAGE_TYPE;
    PiKtures::Utility::applyToEachChannel(
        [rate, bonus](Mat& c){
            c.forEach<uint8_t>([rate, bonus](uint8_t& p, const int* pos){
                p = cv::saturate_cast<uint8_t>(p * rate + bonus);
            });
        }, image, image
    );
}