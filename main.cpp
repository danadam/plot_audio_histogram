#include "mysndfileutils.h"

#include <algorithm>

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <map>
#include <string_view>
#include <vector>



constexpr auto MAX_SAMPLE = std::numeric_limits<double>::max();
const char * g_progname;



using SampleT = double;
using SignalT = std::vector<SampleT>;

const char *
get_filename(const char * path) {
    auto x = strrchr(path, '/');
    return x == nullptr ? path : ++x;
}

#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
[[noreturn]] void
die(const char * progname, const char* fmt, ...) {
    fprintf(stderr, "%s: ", progname);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");
    std::exit(EXIT_FAILURE);
}

struct FileCloser {
    void operator()(FILE * fd) {
        fclose(fd);
    }
};
using FileP = std::unique_ptr<FILE, FileCloser>;



std::pair<SndFileP, SF_INFO>
openfile_read(const char * path) {
    SF_INFO sfinfo{};
    SndFileP sndfile{ sf_open(path, SFM_READ, &sfinfo) };
    if (!sndfile) {
        die(g_progname, "failed to open %s: %s", path, sf_strerror(nullptr));
    }
    printf("%s - %s\n", path, to_string(&sfinfo).c_str());
    return { std::move(sndfile), sfinfo };
}

double
fromdbfs(double dbfs) {
    return pow(10, dbfs / 20);
}

double
todbfs(double val) {
    assert(val > 0);
    return 20 * log(val) / log(10);
}

double
get_window_rms(const SignalT & signal, int start, int win_size) {
    int signal_size = (int)signal.size();
    assert(start < signal_size);

    int signal_remaining = signal_size - start;
    if (win_size > signal_remaining) {
        win_size = signal_remaining;
    }
    double result = 0;
    for (int i = start; i < start+win_size; i++) {
        result += signal[i] * signal[i];
    }

    return std::sqrt(result / win_size);
}



struct Options {
    const char * input_path;
    const char * output_path;
    bool force_overwrite;
};

Options
parse_opts(const int argc, const char * const * argv) {
    using namespace std::literals;
    Options result{};

    int cur = 1;
    while (cur < argc) {
        auto opt = argv[cur];
        if (opt == "-i"sv || opt == "--input"sv) {
            if (cur + 1 >= argc) {
                die(g_progname, "Argument missing for option %s. Try: %s --help", opt, g_progname);
            }
            cur++;
            auto arg = argv[cur];
            result.input_path = arg;
        }
        else if (opt == "-o"sv || opt == "--output"sv) {
            if (cur + 1 >= argc) {
                die(g_progname, "Argument missing for option %s. Try: %s --help", opt, g_progname);
            }
            cur++;
            auto arg = argv[cur];
            result.output_path = arg;
        }
        else if (opt == "-F"sv  || opt == "--force"sv) {
            result.force_overwrite = true;
        }
        cur++;
    }
    return result;
}

void
validate_opts(const Options & opts) {
    if (!opts.input_path) {
        die(g_progname, "input file is required. Try: %s --help", g_progname);
    }
    FileP fp{fopen(opts.input_path, "r")};
    if (!fp) {
        die(g_progname, "can't open %s", opts.input_path);
    }

    if (!opts.output_path) {
        die(g_progname, "output file is required. Try: %s --help", g_progname);
    }
    fp.reset(fopen(opts.output_path, "r"));
    if (fp && !opts.force_overwrite) {
        die(g_progname, "%s already exists. Use --force", opts.output_path);
    }
}

void
print_usage(const char * progname, FILE * out = stdout) {
    fprintf(out,
"Usage:\n"
"  %s -i FILE -o FILE\n"
"\n"
"    -i | --input FILE      - input audio file\n"
"    -o | --output FILE     - output file\n"
"\n"
"Generate histogram of samples and RMS values.\n"
"\n"
"50ms window is used for calculating RMS values. Histogram values are normalized\n"
"so that max value is 100.\n"
"\n"
"Bucket 1 covers all values greater than 0 dBFS, bucket -140 covers all values\n"
"lower or equal -140 dBFS\n",
            progname
        );
}



template<typename KeyT, typename ValueT>
ValueT
find_max_value(const std::map<KeyT, ValueT> & m) {
    assert(!m.empty());
    return std::max_element(
            begin(m),
            end(m),
            [](auto && p1, auto && p2) { return p1.second < p2.second; }
        )->second;
}



struct ChannelData {
    std::map<double, int> hist_pk;
    std::map<double, int> hist_rms;
    int max_num_of_pk;
    int max_num_of_rms;
};

int
main(int argc, char * argv[]) {
    using namespace std::literals;
    g_progname = get_filename(argv[0]);

    if (argc >= 2 && (argv[1] == "-h"sv || argv[1] == "--help"sv)) {
        print_usage(g_progname);
        std::exit(EXIT_SUCCESS);
    }

    auto opts = parse_opts(argc, argv);
    validate_opts(opts);

    sf_hello();

    auto [sndfile1, sfinfo1] = openfile_read(opts.input_path);

    FileP file_out{fopen(opts.output_path, "w")};
    if (!file_out) {
        die(g_progname, "can't open %s for writing", opts.output_path);
    }

    SignalT content_all; content_all.resize(sfinfo1.channels * sfinfo1.frames);
    int nread = sf_read_double(sndfile1.get(), &content_all[0], content_all.size());
    if (nread <= 0) {
        die(g_progname, "no audio read from %s", opts.input_path);
    }
    printf("nread %d (samples)\n", nread);

    int win_50ms = sfinfo1.samplerate / 200;
    printf("50ms rms window has %d frames\n", win_50ms);

    std::map<double, int> hist_template;
    hist_template[MAX_SAMPLE] = 0;
    for (int dbfs = 0; dbfs >= -140; dbfs--) {
        hist_template[fromdbfs(dbfs)] = 0;
    }
    ChannelData ch_data_template{hist_template, hist_template, 0, 0};

    auto num_of_ch = sfinfo1.channels;
    std::vector<ChannelData> ch_data; ch_data.resize(num_of_ch, ch_data_template);
    for (int ch = 0; ch < num_of_ch; ch++) {
        SignalT content; content.resize(nread / num_of_ch);
        for (size_t i = 0; i < content.size(); i++) {
            content[i] = content_all[i * num_of_ch + ch];
        }
        auto num_of_zeros = std::count(begin(content), end(content), 0.0);
        printf("ch %d has %ld zeros\n", ch, num_of_zeros);

        auto & hist_pk = ch_data[ch].hist_pk;
        for (auto && sample : content) {
            hist_pk.lower_bound(std::abs(sample))->second++;
        }

        auto & hist_rms = ch_data[ch].hist_rms;
        for (size_t i = 0; i < content.size(); i += win_50ms) {
            auto rms = get_window_rms(content, i, win_50ms);
            hist_rms.lower_bound(rms)->second++;
        }

        ch_data[ch].max_num_of_pk = find_max_value(hist_pk);
        ch_data[ch].max_num_of_rms = find_max_value(hist_rms);
    }

    // write the header
    fprintf(
            file_out.get(),
"# bucket 1 covers all values greater than 0 dBFS\n"
"# bucket -140 covers all values lower or equal -140 dBFS\n"
"#\n"
"# bucket"
        );
    for (int ch = 0; ch < num_of_ch; ch++) {
        fprintf(file_out.get(), " ch_%d_peak ch_%d_rms", ch, ch);
    }
    fprintf(file_out.get(), "\n");

    // write the data
    for (auto it = rbegin(hist_template); it != rend(hist_template); ++it) {
        auto bucket = it->first;
        fprintf(file_out.get(), "%.1f", (bucket == MAX_SAMPLE ? 1.0 : todbfs(bucket)));

        for (int ch = 0; ch < num_of_ch; ch++) {
            fprintf(
                    file_out.get(),
                    "\t%.1f\t%.1f",
                    100.0 * ch_data[ch].hist_pk[bucket] / ch_data[ch].max_num_of_pk,
                    100.0 * ch_data[ch].hist_rms[bucket] / ch_data[ch].max_num_of_rms
                );
        }
        fprintf(file_out.get(), "\n");
    }
    printf("done\n");
}
