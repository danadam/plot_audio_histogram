#include <memory>
#include <string>

#include <sndfile.h>

struct SndFileCloser {
    void operator()(SNDFILE * fd) {
        sf_close(fd);
    }
};
using SndFileP = std::unique_ptr<SNDFILE, SndFileCloser>;

constexpr const char * sf_major_format_str(int f) {
    switch (f) {
        case SF_FORMAT_WAV:     return "WAV";          // = 0x010000,     /* Microsoft WAV format (little endian). */
        case SF_FORMAT_AIFF:    return "AIFF";         // = 0x020000,     /* Apple/SGI AIFF format (big endian). */
        case SF_FORMAT_AU:      return "AU";           // = 0x030000,     /* Sun/NeXT AU format (big endian). */
        case SF_FORMAT_RAW:     return "RAW";          // = 0x040000,     /* RAW PCM data. */
        case SF_FORMAT_PAF:     return "PAF";          // = 0x050000,     /* Ensoniq PARIS file format. */
        case SF_FORMAT_SVX:     return "SVX";          // = 0x060000,     /* Amiga IFF / SVX8 / SV16 format. */
        case SF_FORMAT_NIST:    return "NIST";         // = 0x070000,     /* Sphere NIST format. */
        case SF_FORMAT_VOC:     return "VOC";          // = 0x080000,     /* VOC files. */
        case SF_FORMAT_IRCAM:   return "IRCAM";        // = 0x0A0000,     /* Berkeley/IRCAM/CARL */
        case SF_FORMAT_W64:     return "W64";          // = 0x0B0000,     /* Sonic Foundry's 64 bit RIFF/WAV */
        case SF_FORMAT_MAT4:    return "MAT4";         // = 0x0C0000,     /* Matlab (tm) V4.2 / GNU Octave 2.0 */
        case SF_FORMAT_MAT5:    return "MAT5";         // = 0x0D0000,     /* Matlab (tm) V5.0 / GNU Octave 2.1 */
        case SF_FORMAT_PVF:     return "PVF";          // = 0x0E0000,     /* Portable Voice Format */
        case SF_FORMAT_XI:      return "XI";           // = 0x0F0000,     /* Fasttracker 2 Extended Instrument */
        case SF_FORMAT_HTK:     return "HTK";          // = 0x100000,     /* HMM Tool Kit format */
        case SF_FORMAT_SDS:     return "SDS";          // = 0x110000,     /* Midi Sample Dump Standard */
        case SF_FORMAT_AVR:     return "AVR";          // = 0x120000,     /* Audio Visual Research */
        case SF_FORMAT_WAVEX:   return "WAVEX";        // = 0x130000,     /* MS WAVE with WAVEFORMATEX */
        case SF_FORMAT_SD2:     return "SD2";          // = 0x160000,     /* Sound Designer 2 */
        case SF_FORMAT_FLAC:    return "FLAC";         // = 0x170000,     /* FLAC lossless file format */
        case SF_FORMAT_CAF:     return "CAF";          // = 0x180000,     /* Core Audio File format */
        case SF_FORMAT_WVE:     return "WVE";          // = 0x190000,     /* Psion WVE format */
        case SF_FORMAT_OGG:     return "OGG";          // = 0x200000,     /* Xiph OGG container */
        case SF_FORMAT_MPC2K:   return "MPC2K";        // = 0x210000,     /* Akai MPC 2000 sampler */
        case SF_FORMAT_RF64:    return "RF64";         // = 0x220000,     /* RF64 WAV file */
        default:                return "UNKNOWN";
    }
}

constexpr const char * sf_subtype_str(int s) {
    switch (s) {
        case SF_FORMAT_PCM_S8:      return "PCM_S8";       // = 0x0001,       /* Signed 8 bit data */
        case SF_FORMAT_PCM_16:      return "PCM_16";       // = 0x0002,       /* Signed 16 bit data */
        case SF_FORMAT_PCM_24:      return "PCM_24";       // = 0x0003,       /* Signed 24 bit data */
        case SF_FORMAT_PCM_32:      return "PCM_32";       // = 0x0004,       /* Signed 32 bit data */

        case SF_FORMAT_PCM_U8:      return "PCM_U8";       // = 0x0005,       /* Unsigned 8 bit data (WAV and RAW only) */

        case SF_FORMAT_FLOAT:       return "FLOAT";        // = 0x0006,       /* 32 bit float data */
        case SF_FORMAT_DOUBLE:      return "DOUBLE";       // = 0x0007,       /* 64 bit float data */

        case SF_FORMAT_ULAW:        return "ULAW";         // = 0x0010,       /* U-Law encoded. */
        case SF_FORMAT_ALAW:        return "ALAW";         // = 0x0011,       /* A-Law encoded. */
        case SF_FORMAT_IMA_ADPCM:   return "IMA_ADPCM";    // = 0x0012,       /* IMA ADPCM. */
        case SF_FORMAT_MS_ADPCM:    return "MS_ADPCM";     // = 0x0013,       /* Microsoft ADPCM. */

        case SF_FORMAT_GSM610:      return "GSM610";       // = 0x0020,       /* GSM 6.10 encoding. */
        case SF_FORMAT_VOX_ADPCM:   return "VOX_ADPCM";    // = 0x0021,       /* Oki Dialogic ADPCM encoding. */

        case SF_FORMAT_G721_32:     return "G721_32";      // = 0x0030,       /* 32kbs G721 ADPCM encoding. */
        case SF_FORMAT_G723_24:     return "G723_24";      // = 0x0031,       /* 24kbs G723 ADPCM encoding. */
        case SF_FORMAT_G723_40:     return "G723_40";      // = 0x0032,       /* 40kbs G723 ADPCM encoding. */

        case SF_FORMAT_DWVW_12:     return "DWVW_12";      // = 0x0040,       /* 12 bit Delta Width Variable Word encoding. */
        case SF_FORMAT_DWVW_16:     return "DWVW_16";      // = 0x0041,       /* 16 bit Delta Width Variable Word encoding. */
        case SF_FORMAT_DWVW_24:     return "DWVW_24";      // = 0x0042,       /* 24 bit Delta Width Variable Word encoding. */
        case SF_FORMAT_DWVW_N:      return "DWVW_N";       // = 0x0043,       /* N bit Delta Width Variable Word encoding. */

        case SF_FORMAT_DPCM_8:      return "DPCM_8";       // = 0x0050,       /* 8 bit differential PCM (XI only) */
        case SF_FORMAT_DPCM_16:     return "DPCM_16";      // = 0x0051,       /* 16 bit differential PCM (XI only) */

        case SF_FORMAT_VORBIS:      return "VORBIS";       // = 0x0060,       /* Xiph Vorbis encoding. */
        default:                    return "UNKNOWN";
    }
}

constexpr const char * sf_endian_str(int e) {
    switch (e) {
        case SF_ENDIAN_FILE:    return "FILE";         // = 0x00000000,   /* Default file endian-ness. */
        case SF_ENDIAN_LITTLE:  return "LITTLE";       // = 0x10000000,   /* Force little endian-ness. */
        case SF_ENDIAN_BIG:     return "BIG";          // = 0x20000000,   /* Force big endian-ness. */
        case SF_ENDIAN_CPU:     return "CPU";          // = 0x30000000,   /* Force CPU endian-ness. */
        default:                return "UNKNOWN";
    }
}

inline std::string sf_format_str(int format) {
    int major_format = format & SF_FORMAT_TYPEMASK;
    int subformat = format & SF_FORMAT_SUBMASK;
    int endian = format & SF_FORMAT_ENDMASK;
    return std::string{"format: "} + sf_major_format_str(major_format) + ", encoding: " + sf_subtype_str(subformat) + ", endian: " + sf_endian_str(endian);
}

inline std::string to_string(SF_INFO * sf_info) {
    return "frames: " + std::to_string(sf_info->frames)
        + ", samplerate: " + std::to_string(sf_info->samplerate)
        + ", channels: " + std::to_string(sf_info->channels)
        + ", format: " +  sf_format_str(sf_info->format) + " (" + std::to_string(sf_info->format) + ")"
        + ", seekable? " + std::to_string(sf_info->seekable);
}

inline void sf_hello() {
    char buffer[128];
    sf_command(NULL, SFC_GET_LIB_VERSION, buffer, sizeof(buffer));
    printf("Using %s\n", buffer);
}
