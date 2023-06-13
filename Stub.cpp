// Stub.cpp
#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <unordered_map>
#include "FileManager.h"

namespace asio = boost::asio;

void ReduceSave(const std::string bucketNum) {
    int exit_code = 0;
    std::unordered_map<std::string, int> word_counts;
    std::string reduceDll_ = "\\libs\\Reducer1.dll";
    std::string reduceFunc_ = "reduceWrapper";
    std::string tempDir_ = "c:\\tempfiles_default";



    HINSTANCE hDLL;               // Handle to DLL
    std::wstring  wDllPath1(reduceDll_.begin(), reduceDll_.end());
    LPCWSTR dllLib = wDllPath1.c_str();
    hDLL = LoadLibrary(dllLib);
    int reducerLoadError = GetLastError();
    if (NULL != hDLL)
    {
#define DLL_CALL __cdecl 
        //#define DLL_CALL __stdcall 
        typedef std::unordered_map<std::string, int>(DLL_CALL* LPFNDLLFUNC1)(const std::string&);
        LPFNDLLFUNC1 lpfnDllReduce = (LPFNDLLFUNC1)GetProcAddress(hDLL, reduceFunc_.c_str());
        int reducerAddressError = GetLastError();
        //  std::cout << lpfnDllReduce << " " << *lpfnDllReduce << "\n";
        if (NULL != lpfnDllReduce)
        {
            word_counts = lpfnDllReduce(tempDir_ + "/bucket" + bucketNum);
            //made it better with lambda function in "SaveOutput"
            //SortOutputByFrequency::SortOutput(word_counts);
            std::string output;
            for (auto& kv : word_counts) {
                output += kv.first + ": " + std::to_string(kv.second) + "\n";

            }
            FileManager fm(tempDir_);
            exit_code = fm.SaveTemp(output);
        }
        else
        {
            // report the error
            std::cout << "can't get the address of the reduce func";
            exit_code = 14;
        }
        FreeLibrary(hDLL);
    }
    else
    {
        std::cout << "cant load the reducer dll";
        exit_code = 15;
    }


}

void createMapperProcess(const std::string& subBuffer) {
    // Implement the logic to create the Mapper child process
    // For example, you can use system commands or process creation functions provided by your operating system or framework
    // Here, we print the process type and sub-buffer received
    std::cout << "Creating Mapper child process" << std::endl;
    std::cout << "Sub-buffer: " << subBuffer << std::endl;
}

void createReducerProcess(const std::string& subBuffer) {
    // Implement the logic to create the Reducer child process
    // For example, you can use system commands or process creation functions provided by your operating system or framework
    // Here, we print the process type and sub-buffer received

    ReduceSave(subBuffer);

    std::cout << "Creating Reducer child process" << std::endl;
    std::cout << "Sub-buffer: " << subBuffer << std::endl;
}

void handleControllerMessage(const std::string& message) {
    // Find the delimiter separating process type and sub-buffer
    size_t delimiterPos = message.find('|');
    if (delimiterPos == std::string::npos) {
        std::cout << "Invalid message format: " << message << std::endl;
        return;
    }

    // Extract the process type and sub-buffer from the message
    std::string processType = message.substr(0, delimiterPos);
    std::string subBuffer = message.substr(delimiterPos + 1);

    // Check the process type to determine the action
    if (processType == "Mapper") {
        createMapperProcess(subBuffer);
    }
    else if (processType == "Reducer") {
        createReducerProcess(subBuffer);
    }
    else {
        std::cout << "Invalid process type: " << processType << std::endl;
    }
}

void startListening(asio::ip::tcp::socket& socket) {
    try {
        // Continuously listen for incoming messages from the controller
        while (true) {
            // Buffer to store the received message
            std::array<char, 128> buffer;

            // Receive the message from the controller
            boost::system::error_code error;
            std::size_t messageLength = socket.read_some(asio::buffer(buffer), error);

            if (error == boost::asio::error::eof) {
                // Connection closed by the controller
                break;
            }
            else if (error) {
                // Error occurred while receiving the message
                throw std::runtime_error("Error receiving message: " + error.message());
            }

            // Convert the received buffer to a string message
            std::string message(buffer.data(), messageLength);

            // Handle the received message from the controller
            handleControllerMessage(message);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    try {
        asio::io_service io_service;
        asio::ip::tcp::acceptor acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8000));

//        asio::ip::tcp::acceptor acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 8000));


        while (true) {
            // Wait for a connection from the controller
            asio::ip::tcp::socket socket(io_service);
            acceptor.accept(socket);

            // Start listening for messages from the controller
            startListening(socket);

            // Close the socket connection
            socket.close();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
