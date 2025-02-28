#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
	cv::Mat img = cv::imread("./stop.png");
	if (img.empty())
	{
		std::cerr << "Error: Unable to load image." << std::endl;
		return -1;
	}

	cv::Mat hsvImg;
	cv::cvtColor(img, hsvImg, cv::COLOR_BGR2HSV);

	cv::Mat maskA, maskB, combinedMask;
	cv::inRange(hsvImg, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), maskA);
	cv::inRange(hsvImg, cv::Scalar(160, 100, 100), cv::Scalar(180, 255, 255), maskB);
	cv::bitwise_or(maskA, maskB, combinedMask);

	cv::Mat filteredImg;
	cv::bitwise_and(img, img, filteredImg, combinedMask);

	cv::imshow("Original", img);
	cv::imshow("Mask", combinedMask);
	cv::imshow("Filtered", filteredImg);

	cv::Mat grayBlur;
	cv::cvtColor(filteredImg, grayBlur, cv::COLOR_HSV2BGR);
	cv::cvtColor(grayBlur, grayBlur, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(grayBlur, grayBlur, cv::Size(9, 9), 2);

	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(grayBlur, circles, cv::HOUGH_GRADIENT, 1.25, grayBlur.rows, 200, 30);

	for (const auto &circ : circles)
	{
		cv::Point center(cvRound(circ[0]), cvRound(circ[1]));
		int radius = cvRound(circ[2]);
		cv::circle(filteredImg, center, 3, cv::Scalar(0, 255, 0), -1, cv::LINE_AA);
		cv::circle(filteredImg, center, radius, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
	}

	cv::imshow("Detected Circles", filteredImg);

	cv::waitKey(0);
	cv::destroyAllWindows();
	return 0;
}
