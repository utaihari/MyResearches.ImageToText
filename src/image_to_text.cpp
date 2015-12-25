#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include <iostream>
#include <string>
#include <boost/foreach.hpp>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <fstream>

namespace fs = boost::filesystem;
using namespace std;

string ImageToString(cv::Mat& image, string& output, const int LEVEL);

int main(int argc, char* argv[]) {
	//第一引数(argv[1])→データセットのディレクトリパス
	//第二引数(argv[2])→出力ディレクトリのパス

	//設定ここから
	const fs::path INPUT_PATH(argv[1]);
	const fs::path OUTPUT_PATH(argv[2]);
	const int QUANTIZED_LEVEL = 8;//量子化レベル（LEVELの３乗根に量子化）通常{8,27,64,125}を利用する
	//設定ここまで

	//データセットディレクトリの中身を再帰的に（すべてのファイルを）調べる
	BOOST_FOREACH(const fs::path& p, make_pair(fs::recursive_directory_iterator(INPUT_PATH),
					fs::recursive_directory_iterator())) {
		if (!fs::is_directory(p)) {

			string file_extension = p.extension().string();

			//それぞれのファイルに対する操作
			if (file_extension == ".jpg" || file_extension == ".gif") {

				cout << p << endl;

				cv::Mat input_image = cv::imread(p.string());

				//もしもoutputフォルダにクラスフォルダがなかったら作成する　ここから
				fs::path output_class_dir(OUTPUT_PATH / p.parent_path().stem());// "/"はパスの連結演算子

				if (!fs::exists(output_class_dir)) {
					fs::create_directories(output_class_dir);
				}
				//ここまで

				string output;
				ImageToString(input_image, output, QUANTIZED_LEVEL);

				string output_txt_dir = output_class_dir.string() + "/"
						+ p.stem().string() + ".txt";
				std::ofstream ofs(output_txt_dir);
				ofs << output;
				ofs.close();
			}
		}
	}
	cout << "finish" << endl;
	return 0;
}
/***
 * @brief 水平スキャンで画像をテキストに変換
 * @note char型の配列を予め確保しておくなどして高速化したほうがいいかも
 * @param image 入力画像
 * @param LEVEL 量子化レベル
 * @return テキスト
 */
string ImageToString(cv::Mat& image, string& output, const int LEVEL) {
	cv::Mat lab_image;
	cv::cvtColor(image, lab_image, CV_BGR2Lab);

	int q = 255.0 / pow(LEVEL, 1.0 / 3.0);
	for (int y = 0; y < lab_image.rows; ++y) {
		for (int x = 0; x < lab_image.cols; ++x) {
			cv::Vec3b lab = lab_image.at<cv::Vec3b>(y, x);
			char lab_char[3];
			for (int i = 0; i < 3; ++i) {
				lab_char[i] = (char)((lab[i] - 1) / q);
			}
			output += (char) ((double) (lab_char[0] * q * q)
					+ (double) (lab_char[1] * q) + (double) (lab_char[2]) + 1); // \0 はヌル文字なので出現しないように+1している :255→-1
		}
	}
	return output;
}
