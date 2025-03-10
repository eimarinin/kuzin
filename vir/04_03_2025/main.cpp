#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

constexpr int GAUSSIAN_KERNEL_SIZE = 5;
constexpr double GAUSSIAN_SIGMA = 1.5;
constexpr int CANNY_THRESHOLD1 = 50;
constexpr int CANNY_THRESHOLD2 = 150;
constexpr double SHAPE_EPSILON_COEFF = 0.03;
constexpr double SQUARE_ASPECT_RATIO_TOLERANCE = 0.15;

std::string detectShape(const std::vector<cv::Point> &contour)
{
	std::vector<cv::Point> approx;
	double epsilon = SHAPE_EPSILON_COEFF * cv::arcLength(contour, true);
	cv::approxPolyDP(contour, approx, epsilon, true);

	switch (approx.size())
	{
	case 3:
		return "Triangle";
	case 4:
	{
		cv::Rect rect = cv::boundingRect(approx);
		double aspect = static_cast<double>(rect.width) / rect.height;
		if (std::fabs(aspect - 1.0) < SQUARE_ASPECT_RATIO_TOLERANCE)
			return "Square";
		return "Rectangle";
	}
	default:
		return (approx.size() > 7) ? "Circle" : "Polygon";
	}
}

int main()
{
	cv::Mat image = cv::imread("./image.png");
	if (image.empty())
	{
		std::cerr << "Error: Could not load image!" << std::endl;
		return -1;
	}

	cv::Mat hsv;
	cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

	// Разбиваем каналы HSV
	std::vector<cv::Mat> hsv_channels;
	cv::split(hsv, hsv_channels);

	// Увеличиваем насыщенность: прибавляем 100 и ограничиваем максимумом 255
	cv::add(hsv_channels[1], cv::Scalar(100), hsv_channels[1]);
	cv::min(hsv_channels[1], 255, hsv_channels[1]);

	cv::merge(hsv_channels, hsv);

	// Используем канал Value для получения grayscale
	cv::Mat gray = hsv_channels[2];

	cv::Mat blurred, edges;
	cv::GaussianBlur(gray, blurred, cv::Size(GAUSSIAN_KERNEL_SIZE, GAUSSIAN_KERNEL_SIZE), GAUSSIAN_SIGMA);
	cv::Canny(blurred, edges, CANNY_THRESHOLD1, CANNY_THRESHOLD2);

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	cv::Mat result = cv::Mat::zeros(image.size(), image.type());
	std::vector<cv::Scalar> colors = {
		cv::Scalar(255, 0, 0),	 // Blue - Triangle
		cv::Scalar(0, 255, 0),	 // Green - Square
		cv::Scalar(0, 255, 255), // Yellow - Rectangle
		cv::Scalar(0, 0, 255)	 // Red - Circle
	};

	for (const auto &contour : contours)
	{
		if (cv::contourArea(contour) < 100)
			continue;

		std::string shape = detectShape(contour);
		cv::Moments m = cv::moments(contour);
		if (m.m00 == 0)
			continue;

		cv::Point center(static_cast<int>(m.m10 / m.m00), static_cast<int>(m.m01 / m.m00));
		cv::Scalar color;
		if (shape == "Triangle")
			color = colors[0];
		else if (shape == "Square")
			color = colors[1];
		else if (shape == "Rectangle")
			color = colors[2];
		else
			color = colors[3];

		cv::drawContours(result, std::vector<std::vector<cv::Point>>{contour}, -1, color, 2);
		cv::putText(result, shape, center, cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
	}

	cv::imshow("Original Image", image);
	cv::imshow("Processing Result", result);
	cv::waitKey(0);

	return 0;
}
