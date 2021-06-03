#include<utility.hpp>
#include<stdexcept>
cv::Mat PiKtures::Utility::transformSpectrum(const Mat& origin){
    if(origin.cols == 1 || origin.rows == 1) throw std::runtime_error("panic: Spectrum is too small.");
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
    if(origin.type() != CV_32FC2) throw std::runtime_error("panic: Spectrum is invalid.");
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