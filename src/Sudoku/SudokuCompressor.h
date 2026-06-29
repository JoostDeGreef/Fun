#pragma once

class SudokuCompressor
{
public:
    SudokuCompressor()
    {
        Clear();
    }
    void Clear()
    {
        data.fill(0);
    }
    // return true if the digit is in range and succesfully added
    bool PushDecimalDigit(const unsigned int digit)
    {
        if (digit > 9)
        {
            return false;
        }
        return PushDigit<10>(digit);
    }
    bool Pushbase64Char(const char c)
    {
        unsigned int digit;
        if (c >= '0' && c <= '9')
        {
            digit = c - '0';
        }
        else if (c >= 'A' && c <= 'Z')
        {
            digit = c - 'A' + 10;
        }
        else if (c >= 'a' && c <= 'z')
        {
            digit = c - 'a' + 36;
        }
        else if (c == '-')
        {
            digit = 62;
        }
        else if (c == '_')
        {
            digit = 63;
        }
        else
        {
            return false;
        }
        return PushDigit<64>(digit);
    }
    int PopDecimalDigit()
    {
        return PopDigit<10>();
    }
    char PopBase64Char()
    {
        static const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
        int res = PopDigit<64>();
        return chars[res];
    }
private:
    template<unsigned int BASE>
    bool PushDigit(const unsigned int digit)
    {
        uint64_t carry = digit;
        for (auto& elem : data)
        {
            uint64_t v = elem;
            v = v * BASE + carry;
            elem = (uint32_t)(v);
            carry = v >> 32;
        }
        return carry == 0;
    }
    template<unsigned int BASE>
    int PopDigit()
    {
        // this divides 'data' by BASE and returns the remainder
        uint32_t R = 0;
        std::array<uint32_t, 10> Q = { 0 };
        // determine highest used bit in data
        int n = 0;
        for (int i = data.size() - 1; i >= 0 && n == 0; --i)
        {
            uint32_t t = data[i];
            if (t)
            {
                n = i * 32;
                while (t >>= 1)
                {
                    n++;
                }
            }
        }
        // divide data by BASE
        for (int i = n; i >= 0; --i)
        {
            R <<= 1;
            int idx = i / 32;
            int bit = i % 32;
            if (data[idx] & (1 << bit))
            {
                R += 1;
            }
            if (R >= BASE)
            {
                R -= BASE;
                Q[idx] |= (1 << bit);
            }
        }
        data.swap(Q);
        return R;
    }
    std::array<uint32_t, 10> data;
};
