#include<rowdy.hpp>
using namespace std;
using namespace cv;
int main(){
    Mat srcImage = imread("smp.jpg");
    if(!srcImage.data){
		cerr<<"Failed to read image.\n";
		return -1;
	}
    Mat salted = srcImage.clone();
    PiKtures::Rowdy::salt(salted, 0.01, 0.01);
    Mat randomed = srcImage.clone();
    PiKtures::Rowdy::random(randomed, 0.02);
    Mat gaussianed = srcImage.clone();
    PiKtures::Rowdy::gaussian(gaussianed, 0.0, 15.0);
    Mat blind_watermarked = srcImage.clone();
    PiKtures::Rowdy::blindWatermark(blind_watermarked, "CuteKitties!", 1, 500);
    Mat deblind_watermarked = blind_watermarked.clone();
    PiKtures::Rowdy::uncoverWatermark(deblind_watermarked);
	imwrite("salted.jpg", salted);
    imwrite("randomed.jpg", randomed);
    imwrite("gaussianed.jpg", gaussianed);
    imwrite("blind_watermarked.jpg", blind_watermarked);
    imwrite("deblind_watermarked.jpg", deblind_watermarked);
    return 0;
}