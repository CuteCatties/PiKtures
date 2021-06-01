#include<rowdy.hpp>
#include<stdexcept>
#include<random>
#include<vector>
namespace PiKtures::Rowdy{
    using __rowdy_pixel_t = cv::Point3_<uint8_t>;
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
    template<typename T>
    static inline bool checkPossibility(const std::vector<T>& possibilities){
        T sum = 0.0;
        for(const auto& x: possibilities){
            if(x >= 1.0 || x <= 0.0) return true;
            if((sum += x) >= 1.0) return true;
        }
        return false;
    }
    static inline bool checkImage(Mat& image){
        if(image.channels() == 4){   // assume that picture with 4 channels HAS RGBA-color
            // cruelly elimate the alpha channel
            cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
        }else if(image.channels() != 3) return true;
        // Of course, we assume that picture with 3 channels HAS RGB-color.
        // NO EXCEPTION
        return false;
    }
}
void PiKtures::Rowdy::salt(Mat& image, double p_black, double p_white){
    if(checkPossibility(std::vector<double>({p_black, p_white}))) throw std::runtime_error("panic: Possibility is invalid.");
    if(checkImage(image)) throw std::runtime_error("panic: Image is invalid.");
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
    if(checkPossibility(std::vector<double>({p_noise}))) throw std::runtime_error("panic: Possibility is invalid.");
    if(checkImage(image)) throw std::runtime_error("panic: Image is invalid.");
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
    if(deviation < 0) throw std::runtime_error("panic: Negative standard deviation is invalid.");
    if(checkImage(image)) throw std::runtime_error("panic: Image is invalid.");
    auto& generator = getOpenCVRandomGenerator();
    Mat noise_map(image.size(), image.type());
    generator.fill(noise_map, cv::RNG::NORMAL, expectation, deviation);
    image += noise_map;
}
void PiKtures::Rowdy::blindWatermark(Mat& image, const char* const watermark, double size_scale){
    if(checkImage(image)) throw std::runtime_error("panic: Image is invalid.");
    Mat channles[4];
    cv::split(image, channles);
    channles[2].convertTo(channles[2], CV_32F);
    channles[3] = Mat::zeros(image.size(), CV_32F);
    Mat operational_matrix;
    cv::merge(channles + 2, 2, operational_matrix);
    cv::dft(operational_matrix, operational_matrix);
    cv::putText(operational_matrix, watermark, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, size_scale, cv::Scalar(255, 255, 255));
    cv::flip(operational_matrix, operational_matrix, -1);
    cv::putText(operational_matrix, watermark, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, size_scale, cv::Scalar(255, 255, 255));
    cv::flip(operational_matrix, operational_matrix, -1);
    cv::idft(operational_matrix, operational_matrix, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
    operational_matrix.convertTo(channles[2], CV_8U);
    cv::merge(channles, 3, image);
}
void PiKtures::Rowdy::uncoverWatermark(Mat& image){
    if(checkImage(image)) throw std::runtime_error("panic: Image is invalid.");
    Mat channles[4];
    Mat buffer[4];
    cv::split(image, channles);
    channles[2].convertTo(channles[2], CV_32F);
    channles[3] = Mat::zeros(image.size(), CV_32F);
    Mat operational_matrix;
    cv::merge(channles + 2, 2, operational_matrix);
    cv::dft(operational_matrix, operational_matrix);
    cv::split(operational_matrix, buffer);
    cv::magnitude(buffer[0], buffer[1], buffer[0]);
    Mat& m = buffer[0];
    cv::add(Mat::ones(m.size(), CV_32F), m, m);
    cv::log(m, m);
    m = m(cv::Rect(0, 0, m.cols & -2, m.rows & -2));
	int half_width = m.cols >> 1;
	int half_height = m.rows >> 1;
	Mat s0(m, cv::Rect(0, 0, half_width, half_height));
	Mat s1(m, cv::Rect(half_width, 0, half_width, half_height));
	Mat s2(m, cv::Rect(0, half_height, half_width, half_height));
	Mat s3(m, cv::Rect(half_width, half_height, half_width, half_height));
	Mat temp_buffer;
	s0.copyTo(temp_buffer);
	s3.copyTo(s0);
	temp_buffer.copyTo(s3);
	s1.copyTo(temp_buffer);
	s2.copyTo(s1);
	temp_buffer.copyTo(s2);
    m.convertTo(m, CV_8UC1);
    cv::normalize(m, m, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    image = m;
}