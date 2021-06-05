#include<enhancer.hpp>
#include<utility.hpp>
#include<vector>
#include<string>
using namespace std;
using namespace cv;
using namespace PiKtures::Enhancer;
int main(){
    Mat srcImage = imread("smp.jpg");
    if(!srcImage.data){
		cerr<<"Failed to read image.\n";
		return -1;
	}
    Mat median = srcImage.clone();
    medianFilter(median, 3);
    imwrite("median.jpg", median);
    Mat mean = srcImage.clone();
    meanFilter(mean, 3);
    imwrite("mean.jpg", mean);
    Mat gauss = srcImage.clone();
    gaussianFilter(gauss, 3, 1.5);
    imwrite("gauss.jpg", gauss);
    Mat bilateral = srcImage.clone();
    bilateralFilter(bilateral, 5, 100);
    imwrite("bilateral.jpg", bilateral);
    Mat butterworth = srcImage.clone();
    butterworthLowpassFilter(butterworth, 500, 30);
    imwrite("butterworth.jpg", butterworth);
    Mat g = srcImage.clone();
    gaussianLowpassFilter(g, 300);
    imwrite("g.jpg", g);
    Mat dermabrasion = srcImage.clone();
    PiKtures::Enhancer::dermabrasion(dermabrasion, 5, 100);
    imwrite("dermabrasion.jpg", dermabrasion);
    Mat luminant = srcImage.clone();
    luminance(luminant, 1.2, 15);
    imwrite("luminant.jpg", luminant);
    return 0;
}