#include "log_policy_file.h"
#include <mutex>

// Since the same file may be opened multiple times, a per-file based
// mutex is needed, thus a common map of file-mutex pairs to all
// instances is needed, with a mutex to protect manipulation of this 
// map.
std::mutex logging::log_policy_file::mutex_map_access;

std::map<std::string, std::unique_ptr<std::mutex>>
    logging::log_policy_file::mutex_map;


void 
logging::log_policy_file::open_ostream(const std::string& file_out)
{
    bool known_file = false;

    std::unique_lock<std::mutex> guard(mutex_map_access);

    // Check if the file was already opened before
    if (mutex_map.find(file_out) != mutex_map.end())
        known_file = true;


    switch (known_file) {
    case true:
        // File was already opened by another logger, do not 
        // discard its content.
        out_stream->open(file_out,
            std::ios_base::binary | std::ios_base::out |
            std::ios_base::app);
        break;

    case false:
        // First time logging to this file, discard its contents 
        // Note: file could not be opened with ios_base::trunc 
        // and ios_base::app at the same time.
        out_stream->open(file_out,
            std::ios_base::binary | std::ios_base::out |
            std::ios_base::trunc);
        out_stream->close();
        out_stream->open(file_out,
            std::ios_base::binary | std::ios_base::out |
            std::ios_base::app);
        break;
    }

    if (out_stream->is_open() == false)
    {
        throw log_exception("logger can't open file " + file_out);
    }

    // If the key already exists in the map does nothing, 
    // otherwise constructs in place map-key and map-value.
    try {
        mutex_map.try_emplace(file_out, std::make_unique<std::mutex>());
    }
    catch (std::bad_alloc e) { throw log_exception(e.what()); }
    catch (std::length_error e) { throw log_exception(e.what()); }

    // Remember file name.
    file_name = file_out;
}



void 
logging::log_policy_file::close_ostream()
{
    std::unique_lock<std::mutex> guard(*(mutex_map.at(file_name)));
    if (out_stream) out_stream->close();
}



void 
logging::log_policy_file::write(const std::string& msg)
{
    std::unique_lock<std::mutex> guard(*(mutex_map.at(file_name)));
    (*out_stream) << msg << std::endl;
}