#include <cstdint>
#include <sys/time.h>

extern "C" {
int xd3_encode_memory(const uint8_t* input,
                      uint32_t input_size,
                      const uint8_t* source,
                      uint32_t source_size,
                      uint8_t* output,
                      uint32_t* output_size,
                      uint32_t output_size_max,
                      int flags);

int xd3_decode_memory(const uint8_t* input,
                      uint32_t input_size,
                      const uint8_t* source,
                      uint32_t source_size,
                      uint8_t* output,
                      uint32_t* output_size,
                      uint32_t output_size_max,
                      int flags);
}

namespace GNET
{

inline int64_t get_msec_now()
{
    timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000 + now.tv_usec / 1000;
}

inline int conver_xdelta3_retcode(int xd3_ret, Octets& output)
{
    switch (xd3_ret)
    {
        case 0:
            return 0;

        default:
            Log::log(LOG_ERR, "conver_xdelta3_retcode xd3_ret:%d(%.*s).", xd3_ret, LOG_AC(output));
            return ERROR_ROAM_DECODE;
    }
}

const unsigned int MAX_DIFF_DATA_BUFF_SIZE = 5 * 1024 * 1024; // 5 MB

inline int DataDiff(const Octets& new_data, const Octets& old_data, Octets& output)
{

    static uint8_t obuf[MAX_DIFF_DATA_BUFF_SIZE];
    unsigned int size = old_data.size(), dsize = new_data.size();
    if (!size || !dsize || std::max(size, dsize) > MAX_DIFF_DATA_BUFF_SIZE) return ERROR_GENERAL;

    int eflags = (1 << 13);
    unsigned int osize = 0;

    int ret = xd3_encode_memory((const uint8_t*)new_data.begin(), new_data.size(), (const uint8_t*)old_data.begin(), old_data.size(), obuf, &osize, MAX_DIFF_DATA_BUFF_SIZE, eflags);

    output.replace(obuf, osize);
    return conver_xdelta3_retcode(ret, output);
}

inline int PatchDiff(const Octets& inputdata, const Octets& old_data, Octets& output)
{
    static uint8_t obuf[MAX_DIFF_DATA_BUFF_SIZE];
    unsigned int dsize = inputdata.size();
    unsigned int size = old_data.size();
    if (!size || !dsize || std::max(size, dsize) > MAX_DIFF_DATA_BUFF_SIZE) return ERROR_GENERAL;
    unsigned int osize = 0;

    int ret = xd3_decode_memory((const uint8_t*)inputdata.begin(), inputdata.size(), (const uint8_t*)old_data.begin(), old_data.size(), obuf, &osize, MAX_DIFF_DATA_BUFF_SIZE, 0);

    output.replace(obuf, osize);
    return conver_xdelta3_retcode(ret, output);
}

inline void test_comp()
{
    Octets src, dst, out;
    Marshal::OctetsStream os, os2;

    for (unsigned int i = 0; i < 4096; i++)
        os << i;

    src = os;

    for (unsigned int i = 0; i < 5500; i++)
    {
        if (i % 5 == 0) continue;
        os2 << i;
    }

    dst = os2;

    int ret = DataDiff(dst, src, out);
    fprintf(stderr, "new data:%d, src data:%d, out:%d, ret:%d\n", (int)dst.size(), (int)src.size(), (int)out.size(), ret);

    if (ret == 0)
    {
        Octets out2;
        ret = PatchDiff(out, src, out2);
        fprintf(stderr, "new data:%d, src data:%d, out:%d, ret:%d, equip:%d\n", (int)out.size(), (int)src.size(), (int)out2.size(), ret, out2 == dst);
    }
}

const int TEST_DATA_MODIFY_TIME = 150;
class RoleMapData
{
    struct RoleData
    {
        int modify_time;
        Octets d;
    };
    typedef std::map<ruid_t, RoleData> ROLE_MAP;

    ROLE_MAP _map;
    int64_t data_size;
    unsigned int data_count;

    int64_t diff_size;
    unsigned int diff_count;

    unsigned int undiff_error;
public:
    RoleMapData()
    {
        data_size = 0;
        data_count = 1;
        diff_size = 0;
        diff_count = 1;
        undiff_error = 0;
    }
    static RoleMapData& GetInstance()
    {
        static RoleMapData _instance;
        return _instance;
    }

    void OnChangeWorld(ruid_t roleid, const Octets& data)
    {
        data_size += data.size();
        data_count++;

        ROLE_MAP::iterator it = _map.find(roleid);
        if (it == _map.end())
        {
            RoleData rdata;
            rdata.modify_time = Timer::GetTime();
            rdata.d = data;
            _map[roleid] = rdata;
            return;
        }

        Octets diff;
        Octets src;
        int ret = DataDiff(data, it->second.d, diff);
        int ret2 = 0;

        if (ret == 0)
        {
            diff_size += diff.size();
            diff_count++;

            ret2 = PatchDiff(diff, it->second.d, src);
        }

        if (ret || ret2 || src != data) undiff_error++;

        Log::log(LOG_DEBUG, "DS::Compress roleid:%ld, data:%zu, diff:%zu, ret:%d, ret2:%d, undiff_error:%d/a:%d/ad:%zu", roleid, data.size(), diff.size(), ret, ret2, undiff_error, (unsigned int)(data_size / data_count), (size_t)(diff_size / diff_count));

        if (undiff_error % 19 == 1 && data_count % 100 == 0)
        {
            Log::log(LOG_ERR, "DS::Compress undiff_error:%d, aver_data_sz:%d, count:%d, ave_diff_sz:%zu, count:%u.", undiff_error, (unsigned int)(data_size / data_count), data_count, (size_t)(diff_size / diff_count), diff_count);
        }

        // TEST_DATA_MODIFY_TIME ����ʱ���ˣ���������
        if (it->second.modify_time + TEST_DATA_MODIFY_TIME <= Timer::GetTime())
        {
            it->second.modify_time = Timer::GetTime();
            it->second.d = data;
        }
    }
};

} // namespace GNET

#endif
