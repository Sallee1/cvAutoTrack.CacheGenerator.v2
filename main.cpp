#include <fmt/core.h>
#include <fmt/color.h>

#include <fstream>
#include <algorithm>
#include <filesystem>

void print_to_hex(const std::string &str);
void remake_one_file(const std::string &exe_file, const std::string &target_path);
void remake_all_files(const std::string &exe_root_dir, const std::string &target_path);

// this application work remake pip.exe binary file
// pip.exe has some hard coded path
// so, this application will change the path to the current path

int main(int argc, char *argv[])
{
    fmt::print(fg(fmt::color::blue_violet) | fmt::emphasis::faint, "[ Remake pip! ]\n");

    // check arguments
    if (argc != 3)
    {
        fmt::print(fg(fmt::color::crimson), "Usage: remake_pip.exe [(exe_file or exe_root_dir) targget_path]\n");
        return 1;
    }

    // get arguments
    std::string file_or_dir = argv[1];
    std::string target_path = argv[2];

    // check file or dir
    std::filesystem::path path(file_or_dir);
    if (std::filesystem::is_directory(path))
    {
        remake_all_files(file_or_dir, target_path);
        return 0;
    }

    // get exe file
    std::string exe_file = file_or_dir;

    // check file
    std::ifstream ifs(exe_file, std::ios::binary);
    if (!ifs.is_open())
    {
        fmt::print(fg(fmt::color::crimson), "Failed to open file: {}\n", exe_file);
        return 1;
    }
    ifs.close();

    // check path
    if (target_path.empty())
    {
        fmt::print(fg(fmt::color::crimson), "Failed to get target path: {}\n", target_path);
        return 1;
    }

    // remake file
    remake_one_file(exe_file, target_path);

    return 0;
}

void print_to_hex(const std::string &str)
{
    for (char c : str)
    {
        if (!std::isprint(static_cast<unsigned char>(c)))
            fmt::print(bg(fmt::color::gold) | fmt::emphasis::bold, "\\x{:02X}", static_cast<unsigned char>(c));
        else
            fmt::print(fg(fmt::color::yellow) | fmt::emphasis::bold, "{}", c);
    }
    fmt::print("\n");
}
std::string format_hex(const std::string &str)
{
    std::string result;
    for (char c : str)
    {
        if (!std::isprint(static_cast<unsigned char>(c)))
            result += fmt::format("\\x{:02X}", static_cast<unsigned char>(c));
        else
            result += c;
    }
    return result;
}

void remake_one_file(const std::string &exe_file, const std::string &target_path)
{
    // open file
    std::ifstream ifs(exe_file, std::ios::binary);
    if (!ifs.is_open())
    {
        fmt::print(fg(fmt::color::crimson), "Failed to open file: {}\n", exe_file);
        return;
    }

    // read file
    std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    // find path flag as "#!"
    std::string path_flag = "#!";
    size_t pos = data.rfind(path_flag);
    if (pos == std::string::npos)
    {
        fmt::print(fg(fmt::color::crimson), "Failed to find path flag: {}\n", path_flag);
        return;
    }
    print_to_hex(data.substr(pos, 10));

    // find begin quotation mark as "#!\""
    bool is_has_begin_quotation_mark = false;
    if (data[pos + path_flag.size()] == '\"')
    {
        is_has_begin_quotation_mark = true;
    }

    // find path end as ".exe\x0A\x0D" or ".exe\"\x0A\x0D"
    bool is_has_end_quotation_mark = false;
    bool is_find_path_end = false;
    size_t pos_end = 0;
    {
        std::string path_end = ".exe\x0A\x0D";
        size_t find_pos_end = data.find(path_end, pos);
        if (find_pos_end != std::string::npos)
        {
            is_find_path_end = true;
            pos_end = find_pos_end;
        }
    }
    {
        std::string path_end = ".exe\x22\x0A\x0D";
        size_t find_pos_end = data.find(path_end, pos);
        is_has_end_quotation_mark = true;
        if (find_pos_end != std::string::npos)
        {
            is_find_path_end = true;
            pos_end = find_pos_end;
        }
    }
    if (!is_find_path_end)
    {
        fmt::print(fg(fmt::color::crimson), "Failed to find path end, begin: {}\n", format_hex(data.substr(pos - 10, 20)));
        return;
    }

    // get old path
    std::string old_path = data.substr(pos + path_flag.size(), pos_end + 4 /* (".exe").size() */ - pos - path_flag.size());
    fmt::print(fg(fmt::color::beige) | fmt::emphasis::bold, "Old path: {}\n", old_path);
    fmt::print(fg(fmt::color::beige) | fmt::emphasis::bold, "New path: {}\n", target_path);

    // replace path
    data.replace(pos + path_flag.size(), old_path.size(), target_path);

    // if has quotation mark, in target insert end quotation mark
    // if (is_has_end_quotation_mark)
    //{
    //    data.insert(pos + path_flag.size() + target_path.size(), "\"");
    //}

    // if has begin quotation mark, in target insert begin quotation mark
    if (is_has_begin_quotation_mark)
    {
        data.insert(pos + path_flag.size(), "\"");
    }

    // write file
    std::ofstream ofs(exe_file, std::ios::binary);
    if (!ofs.is_open())
    {
        fmt::print(fg(fmt::color::crimson), "Failed to open file: {}\n", exe_file);
        return;
    }
    ofs.write(data.c_str(), data.size());
    ofs.close();

    fmt::print(fg(fmt::color::green), "Success to remake {}!\n", exe_file);
}

void remake_all_files(const std::string &exe_root_dir, const std::string &target_path)
{
    // open dir
    std::filesystem::path root_dir(exe_root_dir);
    if (!std::filesystem::exists(root_dir))
    {
        fmt::print(fg(fmt::color::crimson), "Failed to open dir: {}\n", exe_root_dir);
        return;
    }

    // find exe files
    for (const auto &entry : std::filesystem::recursive_directory_iterator(root_dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".exe")
        {
            remake_one_file(entry.path().string(), target_path);
        }
    }
}