#include <opencv2/opencv.hpp>
#include <iostream>

// Функция для добавления текста к изображению
void addLabel(cv::Mat &image, const std::string &label)
{
	cv::putText(image, label, cv::Point(20, 30),
				cv::FONT_HERSHEY_SIMPLEX, 0.7,
				cv::Scalar(255, 255, 255), 2);
}

// Функция для приведения всех изображений к одному формату (3-канальному BGR)
cv::Mat ensureSameFormat(cv::Mat img, const cv::Size &size)
{
	cv::Mat resized;
	cv::resize(img, resized, size); // Приведение к единому размеру

	// Конвертация в BGR, если необходимо
	if (resized.channels() == 1)
	{
		cv::cvtColor(resized, resized, cv::COLOR_GRAY2BGR);
	}
	else if (resized.channels() == 4)
	{
		cv::cvtColor(resized, resized, cv::COLOR_BGRA2BGR);
	}

	return resized;
}

int main()
{
	// Установка кодировки UTF-8 для Windows
	std::system("chcp 65001 > nul");

	// Загрузка изображения
	std::string imagePath = "./image.webp";
	cv::Mat img = cv::imread(imagePath, cv::IMREAD_COLOR);
	if (img.empty())
	{
		std::cerr << "Ошибка: не удалось загрузить изображение!" << std::endl;
		return -1;
	}

	// Клонирование изображения для обработки
	cv::Mat imgProcessed = img.clone();

	// 1. Преобразование в оттенки серого
	cv::Mat gray;
	cv::cvtColor(imgProcessed, gray, cv::COLOR_BGR2GRAY);

	// 2. Применение размытия Гаусса
	cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2, 2);

	// 3. Обнаружение кругов с использованием HoughCircles
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, gray.rows / 8, 100, 30, 0, 0);

	// 4. Отрисовка обнаруженных кругов
	for (const auto &circle : circles)
	{
		cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
		int radius = cvRound(circle[2]);
		cv::circle(imgProcessed, center, 3, cv::Scalar(0, 255, 0), -1, cv::LINE_AA);	 // Центр
		cv::circle(imgProcessed, center, radius, cv::Scalar(255, 0, 0), 2, cv::LINE_AA); // Контур
	}

	// 5. Выделение границ с помощью Canny
	cv::Mat edges;
	cv::Canny(gray, edges, 50, 150, 3);

	// 6. Обнаружение линий с помощью HoughLines
	std::vector<cv::Vec2f> lines;
	cv::HoughLines(edges, lines, 1, CV_PI / 180, 150);

	// 7. Отрисовка обнаруженных линий
	for (const auto &line : lines)
	{
		float rho = line[0], theta = line[1];
		double a = std::cos(theta), b = std::sin(theta);
		double x0 = a * rho, y0 = b * rho;
		cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * a));
		cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * a));
		cv::line(imgProcessed, pt1, pt2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
	}

	// Преобразование в различные цветовые пространства
	cv::Mat grayImg, hsvImg, labImg, yuvImg, xyzImg;
	cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
	cv::cvtColor(img, hsvImg, cv::COLOR_BGR2HSV);
	cv::cvtColor(img, labImg, cv::COLOR_BGR2Lab);
	cv::cvtColor(img, yuvImg, cv::COLOR_BGR2YUV);
	cv::cvtColor(img, xyzImg, cv::COLOR_BGR2XYZ);

	// Подготовка изображений для отображения
	const cv::Size thumbSize(300, 200); // Размер миниатюр
	cv::Mat original = ensureSameFormat(img, thumbSize);
	cv::Mat processed = ensureSameFormat(imgProcessed, thumbSize);
	cv::Mat grayDisplay = ensureSameFormat(grayImg, thumbSize);
	cv::Mat hsvDisplay = ensureSameFormat(hsvImg, thumbSize);
	cv::Mat labDisplay = ensureSameFormat(labImg, thumbSize);
	cv::Mat yuvDisplay = ensureSameFormat(yuvImg, thumbSize);
	cv::Mat xyzDisplay = ensureSameFormat(xyzImg, thumbSize);

	// Добавление текстовых меток
	addLabel(original, "Original");
	addLabel(processed, "Processed");
	addLabel(grayDisplay, "Grayscale");
	addLabel(hsvDisplay, "HSV");
	addLabel(labDisplay, "Lab");
	addLabel(yuvDisplay, "YUV");
	addLabel(xyzDisplay, "XYZ");

	// Сборка сетки изображений
	cv::Mat row1, row2, row3, grid;
	cv::hconcat(original, grayDisplay, row1);
	cv::hconcat(hsvDisplay, labDisplay, row2);
	cv::hconcat(yuvDisplay, xyzDisplay, row3);

	cv::vconcat(row1, row2, grid);
	cv::vconcat(grid, row3, grid);
	// Масштабируем обработанное изображение до ширины 600 пикселей
	cv::Mat processedResized;
	cv::resize(processed, processedResized, cv::Size(600, 200));

	// Добавляем обработанное изображение к общей сетке
	cv::vconcat(grid, processedResized, grid);

	// Отображение
	cv::namedWindow("Image Processing Results", cv::WINDOW_NORMAL);
	cv::imshow("Image Processing Results", grid);
	cv::waitKey(0);

	return 0;
}