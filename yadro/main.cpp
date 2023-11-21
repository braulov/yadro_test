#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cstring>
#ifdef __unix
#define TYPE_SLASH 47
#else
#define TYPE_SLASH 92
#endif
std::streampos next(std::streampos cur)  {return cur.operator+(1);}
std::streampos prev(std::streampos cur)  {return cur.operator-(1);}
struct tape {
protected:
    std::fstream stream;
    std::string file;
    int numbers;
    int digits;
public:
    std::string format_number(int x) {
        std::string s = std::to_string(x);
        int len = s.size();
        for (int i = 0; i < digits - len; i++) {
            s = '0' + s;
        }
        return s;
    }
    void format_file() {
        stream.seekg(0);
        std::fstream tmp_stream("tmp_file.txt", std::ios::in | std::ios::out | std::ios::trunc);
        int count = 0;
        while(true) {
            if (count == numbers) {
                break;
            }
            int number;
            stream >> number;
            std::string s = format_number(number);
            tmp_stream << format_number(number) << ' ';
            count++;
        }
        tmp_stream << '#';
        stream.seekg(0);
        tmp_stream.seekg(0);
        stream << tmp_stream.rdbuf();
        stream.seekp(0);
        tmp_stream.close();
    }
    void reformat_file() {
        stream.seekg(0);
        std::fstream tmp_stream("tmp_file.txt", std::ios::in | std::ios::out | std::ios::trunc);
        int count = 0;
        while(true) {
            int number;
            stream >> number;
            tmp_stream << number;
            if (count < numbers-1) tmp_stream << ' ';
            if (count == numbers - 1) break;
            count++;
        }
        stream.close();
        stream.open(file, std::ios::in | std::ios::out | std::ios::trunc);
        tmp_stream.seekg(0);
        stream << tmp_stream.rdbuf();
        tmp_stream.close();
    }
    tape() = default;
    tape(int numbers, int digits): numbers(numbers), digits(digits) {}
    tape(const std::string& file_tape, int numbers, int digits): numbers(numbers), digits(digits) {
        stream.open(file_tape, std::ios::in | std::ios::out);
        file = file_tape;
    }
    std::fstream& get_stream() {
        return stream;
    }
    std::string& get_file() {
        return file;
    }
    void move_left() {
        if (stream.tellg() == 0) return;
        if (stream.peek() == '#') {
            stream.seekg(prev(stream.tellg()));
        }
        while (std::isdigit(stream.peek())) {
            stream.seekg(prev(stream.tellg()));
        }

        while (std::isspace(stream.peek())) {
            stream.seekg(prev(stream.tellg()));
        }

        while (std::isdigit(stream.peek())) {
            if (stream.tellg() == 0) break;
            stream.seekg(prev(stream.tellg()));
        }
        if (stream.tellg() != 0) stream.seekg(next(stream.tellg()));
    }
    void move_right() {
        while (std::isdigit(stream.peek())) {
            stream.seekg(next(stream.tellg()));
        }
        while (std::isspace(stream.peek())) {
            stream.seekg(next(stream.tellg()));
        }
        if (stream.peek() == '#') {
            move_left();
        }
    }
    int read() {
        std::string number;
        while(isdigit(stream.peek())) {
            number+= char(stream.peek());
            stream.seekg(stream.tellg().operator+(1));
        }
        stream.seekg(stream.tellg().operator-(number.size()));
        return std::stoi(number);
    }
    void write(int x) {
        std::string number = std::to_string(x);
        number = format_number(x);
        stream.seekp(stream.tellg());
        stream.write(number.c_str(),number.size());
        stream.seekp(stream.tellg().operator-(number.size()));
    }
};
struct sort {
private:
    tape input_tape;
    tape output_tape;
    tape buffer_tape1;
    tape buffer_tape2;
    u_int64_t n,m;
public:
    void write_input_in_output() {
        output_tape.get_stream() << input_tape.get_stream().rdbuf();
        input_tape.get_stream().seekg(0);
        buffer_tape1.get_stream() << input_tape.get_stream().rdbuf();
        input_tape.get_stream().seekg(0);
        buffer_tape2.get_stream() << input_tape.get_stream().rdbuf();
        input_tape.get_stream().seekg(0);
        output_tape.get_stream().seekp(0);
        buffer_tape1.get_stream().seekp(0);
        buffer_tape2.get_stream().seekp(0);
    }
    sort(const std::string& filename_input,
         const std::string& filename_output,int n,int m):n(n),m(m) {
        input_tape = tape(filename_input, n, 10);;
        input_tape.format_file();
        output_tape = tape(filename_output, n, 10);
        std::string path_to_tmp1 = std::filesystem::current_path().string() + char(TYPE_SLASH) + "tmp" + char(TYPE_SLASH) + "buffer1.txt";
        std::string path_to_tmp2 = std::filesystem::current_path().string() + char(TYPE_SLASH) + "tmp" + char(TYPE_SLASH) + "buffer2.txt";
        buffer_tape1 = tape(path_to_tmp1, n, 10);
        buffer_tape2 = tape(path_to_tmp2, n, 10);
        write_input_in_output();
    }


    void sift_down(std::vector<int>& numbers, int i, int heap_size) {
        while(2 * i + 1 < heap_size) {
            int left = 2 * i + 1, right = 2 * i + 2;
            int j = left;
            if (right < heap_size && numbers[right] > numbers[left]) j = right;
            if (numbers[i] >= numbers[j]) break;
            std::swap(numbers[i], numbers[j]);
            i = j;
        }
    }
    void build_heap(std::vector<int>& numbers) {
        for (int i = numbers.size()/2; i >=0; i--) sift_down(numbers,i,numbers.size());
    }
    void heap_sort(std::vector<int>& numbers) {
        build_heap(numbers);
        int heap_size = numbers.size();
        for (int i = 0; i < numbers.size();i++) {
            std::swap(numbers[0], numbers[numbers.size() - 1 - i]);
            heap_size--;
            sift_down(numbers,0,heap_size);
        }
    }
    void sort_chunks() {
        for(int i = 0; i < n/m + (n%m!=0); i++) {
            std::vector<int> numbers(std::min(m,n - i*m));
            for(int j  = 0; j < numbers.size(); j++) {
                numbers[j] = output_tape.read();
                if (j == numbers.size()-1) break;
                output_tape.move_right();
                buffer_tape1.move_right();
                buffer_tape2.move_right();
            }
            heap_sort(numbers);
            for(int j  = 0; j < numbers.size(); j++) {
                if (j == numbers.size()-1) break;
                output_tape.move_left();
                buffer_tape1.move_left();
                buffer_tape2.move_left();
            }

            for(int j  = 0; j < m && i*m + j < n; j++) {
                output_tape.write(numbers[j]);
                buffer_tape1.write(numbers[j]);
                buffer_tape2.write(numbers[j]);
                output_tape.move_right();
                buffer_tape1.move_right();
                buffer_tape2.move_right();
            }
        }
        for(int i = 0;i < n; i++) {
            output_tape.move_left();
            buffer_tape1.move_left();
            buffer_tape2.move_left();
        }
    }
    void merge(uint64_t l, uint64_t mid, uint64_t r) {
        uint64_t p1 = l;
        uint64_t p2 = mid;
        while(p1 < mid || p2 < r) {
            if (p1 < mid && p2 < r) {
                int x_l = buffer_tape1.read(), x_r = buffer_tape2.read();
                if (x_l < x_r) {
                    output_tape.write(x_l);
                    buffer_tape1.move_right();
                    output_tape.move_right();
                    p1++;
                } else {
                    output_tape.write(x_r);
                    buffer_tape2.move_right();
                    output_tape.move_right();
                    p2++;
                }
            } else if (p1 == mid) {
                output_tape.write(buffer_tape2.read());
                buffer_tape2.move_right();
                output_tape.move_right();
                p2++;
            } else {
                output_tape.write(buffer_tape1.read());
                buffer_tape1.move_right();
                output_tape.move_right();
                p1++;
            }
        }
    }
    void merge_sort() {
        sort_chunks();
        if (m >= n) return;
        for (u_int64_t sz = 1; sz*m < n; sz*=2) {
            output_tape.get_stream().seekp(0);
            buffer_tape1.get_stream().seekp(0);
            buffer_tape2.get_stream().seekp(0);
            for (u_int64_t i = 0; i < sz*m; i++) {
                buffer_tape2.move_right();
            }
            for (u_int64_t i = 0; i < n - sz*m; i+= 2*sz * m) {
                u_int64_t l = i, mid = i + sz*m, r = std::min(i + 2*sz*m,n);
                merge(i, i + sz*m,std::min(i + 2*sz*m,n));
                for (int k = 0; k < sz*m; k++) {
                    buffer_tape1.move_right();
                    buffer_tape2.move_right();
                }
            }
            output_tape.get_stream().seekp(0);
            buffer_tape1.get_stream().close();
            buffer_tape2.get_stream().close();
            buffer_tape1.get_stream().open(buffer_tape1.get_file(), std::ios::in | std::ios::out | std::ios::trunc);
            buffer_tape2.get_stream().open(buffer_tape2.get_file(), std::ios::in | std::ios::out | std::ios::trunc);

            buffer_tape1.get_stream()<<output_tape.get_stream().rdbuf();
            output_tape.get_stream().seekp(0);
            buffer_tape2.get_stream()<<output_tape.get_stream().rdbuf();
        }

    }
    void sort_input() {
        merge_sort();
        output_tape.reformat_file();
        input_tape.reformat_file();
    }
};
void parse(int argc, char* argv[], int& n, int& m, std::string& input_file, std::string& output_file) {
    for (int i = 0; i < argc; i++) {
        if (std::strcmp(argv[i],"-n") == 0) {
            n = std::atoi(argv[i+1]);
            i++;
        }
        if (std::strcmp(argv[i],"-m") == 0) {
            m = std::atoi(argv[i+1]);
            i++;
        }
        if (std::strcmp(argv[i],"-input") == 0) {
            input_file = argv[i+1];
            i++;
        }
        if (std::strcmp(argv[i],"-output") == 0) {
            output_file = argv[i+1];
            i++;
        }
    }
}
int main(int argc, char* argv[]) {
    int n,m;
    std::string input_file, output_file;
    parse(argc,argv,n,m,input_file,output_file);
    sort sort_tape(input_file,output_file,n,m);
    sort_tape.sort_input();
}
