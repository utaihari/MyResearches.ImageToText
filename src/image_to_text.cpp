#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include <iostream>
#include <string>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;
using namespace std;

const fs::path INPUT_PATH("dataset");
const fs::path OUTPUT_PATH("output");

int main() {
	boost::system::error_code error;
	BOOST_FOREACH(const fs::path& p, std::make_pair(fs::recursive_directory_iterator(INPUT_PATH),
					fs::recursive_directory_iterator())) {
		if (!fs::is_directory(p)) {
			//それぞれのファイルに対する操作

			//もしもoutputフォルダにクラスフォルダがなかったら作成する
			fs::path output_class_dir(OUTPUT_PATH / p.parent_path().stem());
			if (!fs::exists(output_class_dir)) {
				fs::create_directories(output_class_dir);
			}

			cout << "class:" << p.parent_path().stem() << "\n";
			cout << p.filename() << endl;
			fs::path output_file_path(output_class_dir / p.filename());
			try {
				fs::copy_file(p, output_file_path,boost::filesystem::copy_option::overwrite_if_exists);
			} catch (fs::filesystem_error& ex) {
				std::cout << ex.what() << std::endl;
				throw;
			}
		}
	}
}
