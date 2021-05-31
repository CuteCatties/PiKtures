#include<rowdy.hpp>
#include<stdexcept>
#include<random>
#include<vector>
typedef cv::Point3_<uint8_t> __rowdy_pixel_t;
static auto& getMersenneEngine(){
    static std::mt19937_64 engine = std::mt19937_64(std::random_device()());
    return engine;
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
void PiKtures::salt(Mat& image, double p_black, double p_white){
    constexpr double RANDOM_BASE = 1.0;
    constexpr double RANDOM_RARE = 2.0;
    if(checkPossibility(std::vector<double>({p_black, p_white}))) throw std::runtime_error("panic: Possibility is invalid.");
    if(image.channels() == 4){   // assume that picture with 4 channels HAS RGBA-color
        // cruelly elimate the alpha channel
        cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
    }
    else if(image.channels() != 3) throw std::runtime_error("panic: Can't recognize the color space.");
    p_black += RANDOM_BASE;
    p_white = RANDOM_RARE - p_white;
    std::uniform_real_distribution<>distributor(RANDOM_BASE, RANDOM_RARE);
    auto& engine = getMersenneEngine();
    image.forEach<__rowdy_pixel_t>(
        [&engine, &distributor, &p_black, &p_white](__rowdy_pixel_t& p, const int* pos) -> void{
            double random_result = distributor(engine);
            if(random_result <= p_black) p.x = p.y = p.z = 0;
            else if(random_result >= p_white) p.x = p.y = p.z = -1;
        }
    );
}