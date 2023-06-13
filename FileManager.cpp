//FileManager.cpp
#include "FileManager.h"
static char* env_temp;
bool FileManager::is_the_first_instance_ = true;
//two constructors, one taking an output directory, and another taking input, 
//output, and temporary directories.Both constructors take in strings that 
//represent directory paths.These paths are used later by the class functions to readand write files.
FileManager::FileManager(const std::string& output_dir)
    : dir_(output_dir) {}
//the second constructor is used for managing TEMP dir and its cleaning it on teh first use
//after it runs once , first time, it clean all teh temp files from teh previous run
FileManager::FileManager(const std::string& input_dir, const std::string& output_dir, const std::string& temp_dir)
    : output_dir_(output_dir) , temp_dir_(temp_dir), input_dir_(input_dir) {

    if (is_the_first_instance_) {
        fs::path dir(temp_dir_);
        //temporary , to be removed 
        //boost::filesystem::remove_all(temp_dir_);
        boost::filesystem::create_directory(temp_dir_);

        if (fs::is_directory(dir)) {
            for (fs::recursive_directory_iterator it(dir); it != fs::recursive_directory_iterator(); ++it) {
                if (fs::is_regular_file(it->status())) {
                    fs::remove(it->path());
                }
            }
        }        is_the_first_instance_ = false;
        //then sets the flag for teh first run to false, so no more fiel deletions
    }
}


int FileManager::CreateBucket(const std::string& name) {
    boost::filesystem::create_directory(dir_ + name);
    return 0;
}

int FileManager::DeleteBucket(const std::string& name) {
    boost::filesystem::remove_all(dir_ + name);
    return 0;
}


int FileManager::GetInput(const std::string& file,  std::string& buffer) {
    std::ostringstream oss;
    std::string word;
    int words_read = 0;

    //    std::cout << " from GetInput " << words_read << word << file << " \n";
     

   std::ifstream ifs(file);
        if (!ifs.is_open()) {
            BOOST_LOG_TRIVIAL(warning) << "Warning - Error opening file   " << file;

            return 0;
        }

        while (ifs >> word ) {
            oss << word << " ";
            words_read++;
        }

        ifs.close();
    

    buffer = oss.str();

   // std::cout << " from total GetInput " << total_words_read << buffer << " \n";

    return words_read;
}




//takes a string representing the output and saves it to a file named 
//"output.txt" in the specified output directory. 
int FileManager::SaveOutput(const std::string& output) {
    std::istringstream input_stream(output);
    std::vector<std::pair<std::string, int>> word_counts;

    std::string line;
    while (std::getline(input_stream, line)) {
        std::istringstream line_stream(line);
        std::string word;
        int count;
        line_stream >> word >> count;
        //std::cout << "Word " << word << " count " << count << "\n";
        word_counts.emplace_back(word, count);
    }

    //Sorting word_counts from begin to teh end, The comparator function is a lambda function, defined using the [] syntax. 
    //It takes two parameters, a and b, which are the elements being compared during sorting. 
    //The auto keyword is used to let the compiler deduce the type of a and b automatically.

   std::sort(word_counts.begin(), word_counts.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
        });

    fs::path output_path(dir_);
   
    output_path /= "output.txt";
    //construct the full path to teh output file
    std::ofstream output_file(output_path.string());
    //if teh file us unable to open, locked, perms, etc, fail, Return Error caode 3, and later 
    //it will be saved in the specified TEMPFILE dir.
    if (!output_file.is_open()) {
        std::cerr << "Error creating output file " << output_path << std::endl;
        BOOST_LOG_TRIVIAL(error) << "Error creating output file " << output_path;

        return 3;
    }
    for (const auto& word_count : word_counts) {
        //prints the pairs one by one in a file
        output_file << word_count.first << " " << word_count.second << "\n";
    }
    output_file.close();
    return 0;
}

//save the exit status , the result , Success or Fail string
void FileManager::SaveResult(const std::string& result) {
    fs::path output_path(dir_);

    output_path /= "result.txt";
    std::ofstream output_file(output_path.string());
    if (!output_file.is_open()) {
        std::cerr << "Error creating result file " << output_path << std::endl;
        BOOST_LOG_TRIVIAL(error) << "Error creating result file " << output_path;

        return;
    }
    output_file << result << "\n";
    output_file.close();
}



int FileManager::SaveTemp(const std::string& buffer ) {
    fs::path temp_path(dir_);
       // Generate a random unique file name using the uuid library
    boost::uuids::random_generator gen;
    std::string rand = boost::uuids::to_string(gen());

    // Generate a unique file name based on the current timestamp and a counter
    std::time_t now = std::time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
  //  std::cout << " from Save Temp \n";
    std::stringstream file_name_ss;
    file_name_ss << std::put_time(&timeinfo, "%Y%m%d-%H%M%S") << "-" << rand << ".tmp";
    std::string file_name = file_name_ss.str();

    temp_path /= file_name;
    std::ofstream temp_file(temp_path.string());
    if (!temp_file.is_open()) {
        std::cerr << "Error creating temp file " << temp_path << std::endl;
        BOOST_LOG_TRIVIAL(error) << "Error creating temp file   " << temp_path;

        return 9;
    }
    temp_file << buffer << "\n";
    temp_file.close();

    // Increment the temporary file counter
    return 0;
}




int FileManager::CheckDirs() {

    fs::path input_path(input_dir_);
    if (!fs::exists(input_path)) {
        std::cerr << "Input directory does not exist: " << input_dir_ << std::endl;
        BOOST_LOG_TRIVIAL(error) << "Input directory does not exist: " << input_dir_;
        return 5;
    }

    bool has_nonempty_file = false;
    for (const auto& entry : fs::directory_iterator(input_path)) {
        const auto& status = entry.status();
        if (fs::is_regular_file(status) && fs::file_size(entry) > 0) {
            has_nonempty_file = true;
            break;
        }
    }

    if (!has_nonempty_file) {
        std::cerr << "Input directory contains no readable files with data: " << input_dir_ << std::endl;
        BOOST_LOG_TRIVIAL(error) << "Input directory contains no readable files with data: " << input_dir_;
        return 5;
    }

    fs::path output_path(output_dir_);
    if (!fs::exists(output_path)) {
        std::cerr << "Output directory does not exist: " << output_dir_ << std::endl;
        BOOST_LOG_TRIVIAL(error) << "Output directory does not exist: " << output_dir_;
        return 8;
    }

    fs::path temp_path(temp_dir_);
    if (!fs::exists(temp_path)) {
        std::cerr << "Temp directory does not exist: " << temp_dir_ << std::endl;
        BOOST_LOG_TRIVIAL(error) << "Temp directory does not exist: " << temp_dir_;
        return 7;
            }

    return 0;


}
