#include<utility.hpp>
#include<stdexcept>
const char* PiKtures::Utility::errorMessage(const unsigned int errnum){
    constexpr const char* error_message[] = {
        "OK.",
        "No such command.",
        "Ambiguous command.",
        "Peremeters provided is not enough.",
        "Too many peremeters provided.",
        "Kernel size can't be even.",
        "Kernel size can't be odd.",
        "Kernel is too big.",
        "Kernel is too small.",
        "Cannot recognize the image, must be RGB or RGBA.",
        "Each and every probability must be in range [0, 1].",
        "Summation of probabilities must not exceed 1.",
        "The image is too small in one or both dimension.",
        "The diameter must be positive.",
        "Empty command. You need to say something.",
        "Deviation must not be negative.",
        "The watermark is too big.",
        "Failed to load specified module.",
        "No module loaded.",
        "No image is opened."
    };
    static_assert(static_cast<unsigned int>(ErrorCode::ERRORCODERARE) == sizeof(error_message) / sizeof(const char*));
    if(errnum >= static_cast<unsigned int>(ErrorCode::ERRORCODERARE)) throw std::logic_error("panic: error number out of range.");
    if(errnum == 0) throw std::logic_error("panic: error message of OK shall not be accessed.");
    return error_message[errnum];
}
cv::Mat PiKtures::Utility::calculateSpectrum(const Mat& origin, int channel){
    if(channel >= 3) throw std::logic_error("panic: channel index unacceptable.");
    Mat image = origin.clone();
    checkImage(image);
    Mat channles[4];
    cv::split(image, channles);
    channles[channel].convertTo(channles[channel], CV_32F);
    channles[channel + 1] = Mat::zeros(image.size(), CV_32F);
    Mat operational_matrix;
    cv::merge(channles + channel, 2, operational_matrix);
    cv::dft(operational_matrix, operational_matrix);
    return visualizeSpectrum(transformSpectrum(operational_matrix));
}
cv::Mat PiKtures::Utility::transformSpectrum(const Mat& origin){
    if(origin.cols == 1 || origin.rows == 1) throw ErrorCode::IMAGE_TOO_SMALL;
    Mat result = ((origin.cols & 1) | (origin.rows & 1)) ? alignImage(origin) : origin;
    int half_width = result.cols >> 1;
	int half_height = result.rows >> 1;
    Mat s0(result, cv::Rect(0, 0, half_width, half_height));
	Mat s1(result, cv::Rect(half_width, 0, half_width, half_height));
	Mat s2(result, cv::Rect(0, half_height, half_width, half_height));
	Mat s3(result, cv::Rect(half_width, half_height, half_width, half_height));
	Mat temp_buffer;
	s0.copyTo(temp_buffer);
	s3.copyTo(s0);
	temp_buffer.copyTo(s3);
	s1.copyTo(temp_buffer);
	s2.copyTo(s1);
	temp_buffer.copyTo(s2);
    return result;
}
cv::Mat PiKtures::Utility::visualizeSpectrum(const Mat& origin){
    if(origin.type() != CV_32FC2) throw std::logic_error("panic: Spectrum is invalid.");
    Mat buffer[2];
    cv::split(origin, buffer);
    Mat& result = buffer[0];
    cv::magnitude(result, result, result);
    cv::add(Mat::ones(result.size(), CV_32F), result, result);
    cv::log(result, result);
    result.convertTo(result, CV_8UC1);
    cv::normalize(result, result, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    return result;
}