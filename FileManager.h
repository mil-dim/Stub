//FileManager.h
#ifndef OUTPUTSAVER_H
#define OUTPUTSAVER_H


#include <iomanip> // for std::put_time
#include <ctime>   // for std::time
#include <sstream> // for std::stringstream
#include <string>  // for std::string
#include <fstream> // for std::ofstream
#include <iostream>// for std::cerr
#include <vector>  // for std::vector
#include <utility> // for std::pair
#include <algorithm> // for std::sort
#include <boost/filesystem.hpp> // for boost::filesystem
#include <boost/uuid/uuid.hpp> // for boost::uuids::uuid
#include <boost/uuid/uuid_generators.hpp> // for boost::uuids::random_generator
#include <boost/uuid/uuid_io.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace keywords = boost::log::keywords;

namespace logging = boost::log;





namespace fs = boost::filesystem;

class FileManager {
public:
    FileManager(const std::string& output_dir);
    FileManager(const std::string& input_dir, const std::string& output_dir, const std::string& temp_dir);

    int SaveOutput(const std::string& output);
    void SaveResult(const std::string& result);
    int  SaveTemp(const std::string& buffer);
    int CheckDirs();
    int GetInput(const std::string& file, std::string& buffer);
    int CreateBucket(const std::string& name);
    int DeleteBucket(const std::string& name);


private:
    std::string dir_;
    std::string output_dir_;
    std::string input_dir_;
    std::string temp_dir_;
    //using this var to persist during the program run and indicate the first use,
    //to clean the temp dir
    static bool is_the_first_instance_;


};

#endif // OUTPUTSAVER_H
