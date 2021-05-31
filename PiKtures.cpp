#include<rowdy.hpp>
using namespace std;
using namespace cv;
using namespace PiKtures;
int main(){
    Mat srcImage = imread("smp.jpg");
    if(!srcImage.data){
		cerr<<"Failed to read image.\n";
		return -1;
	}
    salt(srcImage, 0.01, 0.01);
	imwrite("salted.jpg", srcImage);
    return 0;
}