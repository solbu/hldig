#include "HtStdHeader.h"




//
// this function is designed to convert SMALL char*
// strings from utf8 to wide characters. be careful
// about using it to convert long strings, since
// this function is O(2n) on the number of characters 
//
wchar_t * utf8_to_wchar(const char* value)
{
    const char * counter;
    int len = 0;

    //
    // Find the number of characters in the stream.
    //
    counter = value;

    while (counter[0]) {
        if ((counter[0] & 0x80) == 0x00)
        {
            len++;
            counter++;
        }
        else if ((counter[1] & 0xC0) == 0x80)
        {
            if ((counter[0] & 0xE0) == 0xC0)
            {
                len++;
                counter += 2;
            }
            else if (((counter[2] & 0xC0) == 0x80) && ((counter[0] & 0xF0) == 0xE0))
            {
                len++;
                counter += 3;
            }
            else
            {
                counter++; // invalid character
            }
        }
        else
        {
            counter++; // invalid character
        }
    }

    //
    // create a wchar copy of the string
    //
    wchar_t *ucs2_str = (wchar_t *)calloc(len+1, sizeof(wchar_t));
    wchar_t *ucs2_ptr = ucs2_str;
    counter = value;

    while (counter[0])
    {
        if ((counter[0] & 0x80) == 0x00)
        {
            *ucs2_ptr++ = *counter++;
        }
        else if ((counter[1] & 0xC0) == 0x80) 
        {
            if ((counter[0] & 0xE0) == 0xC0)
            {
                *ucs2_ptr++ = ((counter[0] & 0x1F) << 6) | (counter[1] & 0x3F);
                counter += 2;
            }
            else if (((counter[2] & 0xC0) == 0x80) && ((counter[0] & 0xF0) == 0xE0))
            {
                *ucs2_ptr++ = ((counter[0] & 0x0F) << 12) |
                    ((counter[1] & 0x3F) << 6) |
                    (counter[2] & 0x3F);
                counter += 3;
            }
            else
            {
                counter++; // invalid character
            }
        }
        else
        {
            counter++; // invalid character
        }
    }
    *ucs2_ptr = '\0';

    return ucs2_str;
}


char * wchar_to_utf8(const wchar_t* value)
{
    const wchar_t * counter;
    int len = 0;

    //
    // Find the number of characters in the stream.
    //
    counter = value;

    while (*counter)
    {
        if ((*counter & 0xFF80) == 0x0000)
        {
            len++;
        }
        else if ((*counter & 0xF800) == 0x0000)
        {
            len += 2;
        }
        else
        {
            len += 3;
        }
        counter++;
    }

    //
    // create a char copy of the string
    //
    char *char_str = (char *)calloc(len+1, sizeof(char));
    char *char_ptr = char_str;
    counter = value;

    while (*counter)
    {
        if ((*counter & 0xFF80) == 0x0000)
        {
            *char_ptr++ = (*counter & 0x7F);
        }
        else if ((*counter & 0xF800) == 0x0000)
        {
            *char_ptr++ = ((*counter >> 6) & 0x1F) | 0xC0;
            *char_ptr++ = (*counter & 0x3F) | 0x80;
        }
        else
        {
            *char_ptr++ = ((*counter >> 12) & 0x0F) | 0xE0;
            *char_ptr++ = ((*counter >> 6) & 0x3F) | 0x80;
            *char_ptr++ = (*counter & 0x3F) | 0x80;
        }
        counter++;
    }
    *char_ptr = '\0';

    return char_str;
}


