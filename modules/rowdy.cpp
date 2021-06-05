#include<rowdy.hpp>
#include<utility.hpp>
#include<stdexcept>
#include<random>
#include<vector>
namespace PiKtures::Rowdy{
    using __rowdy_pixel_t = cv::Point3_<uint8_t>;
    using PiKtures::Utility::checkProbability;
    using PiKtures::Utility::checkImage;
    using PiKtures::Utility::ErrorCode;
    constexpr double POSSIB_RANDOM_BASE = 1.0;
    constexpr double POSSIB_RANDOM_RARE = 2.0;
    constexpr uint8_t CHANNEL_MIN = 0;
    constexpr uint8_t CHANNEL_MAX = -1;
    static_assert((POSSIB_RANDOM_RARE - POSSIB_RANDOM_BASE - 1.0) <= 1e-6 && (POSSIB_RANDOM_RARE - POSSIB_RANDOM_BASE - 1.0) >= -1e-6);
    static_assert(CHANNEL_MAX > CHANNEL_MIN);
    static auto& getMersenneEngine(){
        static std::mt19937_64 engine = std::mt19937_64(std::random_device()());
        return engine;
    }
    static auto& getOpenCVRandomGenerator(){
        static cv::RNG generator = cv::RNG(std::random_device()());
        return generator;
    }
}
void PiKtures::Rowdy::salt(Mat& image, double p_black, double p_white){
    checkProbability(std::vector<double>({p_black, p_white}));
    checkImage(image);
    p_black += POSSIB_RANDOM_BASE;
    p_white = POSSIB_RANDOM_RARE - p_white;
    std::uniform_real_distribution<>distributor(POSSIB_RANDOM_BASE, POSSIB_RANDOM_RARE);
    auto& engine = getMersenneEngine();
    image.forEach<__rowdy_pixel_t>(
        [&engine, &distributor, &p_black, &p_white](__rowdy_pixel_t& p, const int* pos) -> void{
            double random_result = distributor(engine);
            if(random_result <= p_black) p.x = p.y = p.z = CHANNEL_MIN;
            else if(random_result >= p_white) p.x = p.y = p.z = CHANNEL_MAX;
        }
    );
}
void PiKtures::Rowdy::random(Mat& image, double p_noise){
    checkProbability(std::vector<double>({p_noise}));
    checkImage(image);
    p_noise += POSSIB_RANDOM_BASE;
    std::uniform_real_distribution<>real_distributor(POSSIB_RANDOM_BASE, POSSIB_RANDOM_RARE);
    std::uniform_int_distribution<>int_distributor(CHANNEL_MIN, CHANNEL_MAX);
    auto& engine = getMersenneEngine();
    image.forEach<__rowdy_pixel_t>(
        [&engine, &real_distributor, &int_distributor, &p_noise](__rowdy_pixel_t& p, const int* pos) -> void{
            if(real_distributor(engine) > p_noise) return;
            p.x = int_distributor(engine);
            p.y = int_distributor(engine);
            p.z = int_distributor(engine);
        }
    );
}
void PiKtures::Rowdy::gaussian(Mat& image, double expectation, double deviation){
    if(deviation < 0) throw ErrorCode::NEGATIVE_DEVIATION;
    checkImage(image);
    auto& generator = getOpenCVRandomGenerator();
    Mat noise_map(image.size(), image.type());
    generator.fill(noise_map, cv::RNG::NORMAL, expectation, deviation);
    image += noise_map;
}
void PiKtures::Rowdy::blindWatermark(Mat& image, const char* const watermark, double size_scale, double power){
    const static cv::Scalar color(255, 255, 255);
    checkImage(image);
    Mat channles[4];
    cv::split(image, channles);
    channles[2].convertTo(channles[2], CV_32F);
    channles[3] = Mat::zeros(image.size(), CV_32F);
    Mat operational_matrix;
    cv::merge(channles + 2, 2, operational_matrix);
    cv::dft(operational_matrix, operational_matrix);
    int place_holder;
    cv::Size watermark_size = cv::getTextSize(watermark, cv::FONT_HERSHEY_SIMPLEX, size_scale, 1, &place_holder);
    Mat text_matrix = PiKtures::Utility::alignImage(operational_matrix);
    Mat real_matrix(Mat::zeros(text_matrix.size(), text_matrix.type()));
    cv::Point locate((real_matrix.cols >> 1) - watermark_size.width, (real_matrix.rows >> 1) - watermark_size.height);
    if(locate.x < 0 || locate.y < 0) throw ErrorCode::WATERMARK_TOO_BIG;
    cv::putText(real_matrix, watermark, locate, cv::FONT_HERSHEY_SIMPLEX, size_scale, color);
    cv::flip(real_matrix, real_matrix, -1);
    cv::putText(real_matrix, watermark, locate, cv::FONT_HERSHEY_SIMPLEX, size_scale, color);
    cv::flip(real_matrix, real_matrix, -1);
    cv::scaleAdd(real_matrix, power, text_matrix, text_matrix);
    cv::idft(operational_matrix, operational_matrix, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
    operational_matrix.convertTo(channles[2], CV_8U);
    cv::merge(channles, 3, image);
}
void PiKtures::Rowdy::uncoverWatermark(Mat& image){
    checkImage(image);
    Mat channles[4];
    cv::split(image, channles);
    channles[2].convertTo(channles[2], CV_32F);
    channles[3] = Mat::zeros(image.size(), CV_32F);
    Mat operational_matrix;
    cv::merge(channles + 2, 2, operational_matrix);
    cv::dft(operational_matrix, operational_matrix);
    image = PiKtures::Utility::visualizeSpectrum(PiKtures::Utility::transformSpectrum(operational_matrix));
}