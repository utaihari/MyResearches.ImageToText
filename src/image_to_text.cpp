#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include <iostream>
#include <string>
#include <boost/foreach.hpp>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <string.h>
#include <fstream>

namespace fs = boost::filesystem;
using namespace std;

void ImageToString(cv::Mat& image, char* output, const int LEVEL);

int main(int argc, char* argv[]) {
	//第一引数(argv[1])→データセットのディレクトリパス
	//第二引数(argv[2])→出力ディレクトリのパス

	//設定ここから
	const fs::path INPUT_PATH(argv[1]);
	const fs::path OUTPUT_PATH(argv[2]);
	const int QUANTIZED_LEVEL = 5;	//量子化レベル（LEVELの３乗に量子化）通常{2,3,4,5}を利用する
	//設定ここまで

	//データセットディレクトリの中身を再帰的に（すべてのファイルを）調べる
	BOOST_FOREACH(const fs::path& p, make_pair(fs::recursive_directory_iterator(INPUT_PATH),
					fs::recursive_directory_iterator())){
	if (!fs::is_directory(p)) {

		string file_extension = p.extension().string();

		//それぞれのファイルに対する操作
		if (file_extension == ".jpg" || file_extension == ".gif"
				|| file_extension == ".png") {

			//処理中の画像ファイル名の出力
			cout << p << endl;

			//画像ファイルの読み込み
			cv::Mat input_image = cv::imread(p.string());

			//保存先フォルダの作成
			string output_dir = p.string();
			output_dir.erase(output_dir.begin(),
					output_dir.begin() + INPUT_PATH.string().length());

			//もしもoutputフォルダにクラスフォルダがなかったら作成する　ここから
			fs::path output_class_dir(
					OUTPUT_PATH / fs::path(output_dir).parent_path());// "/"はパスの連結演算子

			if (!fs::exists(output_class_dir)) {
				fs::create_directories(output_class_dir);
			}
			//ここまで

			//出力用のテキスト配列を確保（１画素が１文字になるのでサイズはinput_image.rows * input_image.cols）
			char* output;
			output = new char[input_image.rows * input_image.cols
			+ 1];

			//メインの処理
			ImageToString(input_image, output, QUANTIZED_LEVEL);

			//出力ファイル名の生成
			string output_txt_dir = output_class_dir.string() + "/"
			+ p.stem().string() + ".txt";

			//ファイルにoutputを出力
			std::ofstream ofs(output_txt_dir,std::ios_base::out |ios_base::binary);
			ofs.write(output,strlen(output));
			if(ofs.bad()) {
				cout << "error" << endl;
				break;
			}

			ofs.close();
			delete output;
		}
	}
}
	cout << "finish" << endl;
	return 0;
}
/***
 * @brief 水平スキャンで画像をテキストに変換
 * @param image 入力画像
 * @param LEVEL 量子化レベル
 * @param output 戻り値
 */
void ImageToString(cv::Mat& image, char* output, const int LEVEL) {
	//cv::Mat rgb_image;
	//cv::cvtColor(image, rgb_image, CV_BGR2Lab);	//rgb -> Lab

	int q = ceil(255.0 / LEVEL);

	for (int y = 0; y < image.rows; ++y) {
		for (int x = 0; x < image.cols; ++x) {
			cv::Vec3b rgb = image.at<cv::Vec3b>(y, x);
			char rgb_char[3];

			for (int i = 0; i < 3; ++i) {
				rgb_char[i] = (char) ((rgb[i] - 1) / q);	//量子化
			}
			char c = (char) ((rgb_char[2] * pow(LEVEL, 2))
					+ (rgb_char[1] * LEVEL) + (rgb_char[0]));

			// \0 はヌル文字なので出現しないようにしている
			if (c == '\0') {
				c = 127;
			}

//			//'10' = LF
//			if (c == 10) {
//				c = 1;
//			}
//			//'13' = CR
//			else if (c == 13) {
//				c = 2;
//			}

			output[x + image.cols * y] = c;

		}
	}
	output[image.rows * image.cols] = '\0';
}
