#include "utils.h"
#include "get_device_addr.h"
#include "md5.h"
#include "log.h"
#include <stdlib.h>
#include <arpa/inet.h>

// We don't know why some version of OpenWrt defines LITTLE ENDIAN but actually use BIG ENDIAN.
#ifdef OPENWRT
    #define TO_LITTLE_ENDIAN(n) (((((unsigned long)(n) & 0xFF)) << 24) |        \
                                ((((unsigned long)(n) & 0xFF00)) << 8) |        \
                                ((((unsigned long)(n) & 0xFF0000)) >> 8) |      \
                                ((((unsigned long)(n) & 0xFF000000)) >> 24))
#endif

std::vector<uint8_t> get_md5_digest(std::vector<uint8_t>& data) {
    md5_byte_t digest[16];
    md5_state_t state;
    
    md5_init(&state);
    md5_append(&state, &data[0], (int) data.size());
    md5_finish(&state, digest);
    
    return std::vector<uint8_t>(digest, digest + 16);
}

std::string hex_to_str(uint8_t *hex, size_t len, char separator) {
    char buf[1024] = {0};
    for (size_t i = 0; i < len; i++)
        sprintf(buf + strlen(buf), "%02x%c", hex[i], (i < len - 1) ? separator : 0);
    
    return std::string(buf);
}

void hex_dump(std::vector<uint8_t> hex) {
    char buf[1024];
    
    for (size_t i = 0; i < hex.size(); i += 16)
    {
        sprintf(buf, "%08x: ", (int)i);
        for (int j = 0; j < 16; j++)
        {
            if (i + j < hex.size())
                sprintf(buf + strlen(buf), "%02x ", hex[i+j]);
            else
                strcat(buf, "   ");
            
            if (j == 7) strcat(buf, " ");
        }
        
        strcat(buf, " ");
        for (int j = 0; j < 16; j++)
            if (i + j < hex.size())
                sprintf(buf + strlen(buf), "%c", isprint(hex[i+j]) ? hex[i+j] : '.');
        
        #ifdef SGUDRCOM_DEBUG
                std::clog << buf << std::endl;
            #ifdef SGUDRCOM_PRINT_DBG_ON_SCREEN
                std::cout << buf << std::endl;
            #endif
        #endif
    }
}

std::vector<std::string> split_string(std::string src, char delimiter, bool append_last) {
    std::string::size_type pos = 0;
    std::vector<std::string> ret;
    
    while ((pos = src.find(delimiter)) != std::string::npos)
    {
        ret.push_back(src.substr(0, pos));
        src = src.substr(pos + 1);
    }
    
    // the last element
    if (append_last) ret.push_back(src);
    
    return ret;
}

std::vector<uint8_t> str_ip_to_vec(std::string ip) {
    std::vector<uint8_t> ret(4, 0);
    
    auto vec_addr = split_string(ip, '.');
    if (vec_addr.size() < 4)
        return ret;
    
    unsigned long addr = (atol(vec_addr[0].c_str()) << 24) + (atol(vec_addr[1].c_str()) << 16) + (atol(vec_addr[2].c_str()) << 8) + atol(vec_addr[3].c_str());
    addr = ntohl(addr);
    
    memcpy(&ret[0], &addr, 4);
    
    return ret;
}

std::vector<uint8_t> str_mac_to_vec(std::string mac) {
    std::vector<uint8_t> ret;
    
    auto chartohex = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9')
            return c - '0';
        
        if (c >= 'a' && c <= 'f')
            return (c - 'a') + 0x0a;
        
        if (c >= 'A' && c <= 'F')
            return (c - 'A') + 0x0a;
        
        return 0xFF;
    };
    
    for (int i = 0; i <= 15; i += 3)
    {
        uint8_t b = (chartohex(mac[i]) << 4) + chartohex(mac[i+1]);
        ret.push_back(b);
    }
    
    return ret;
}

std::vector<uint8_t> str_to_vec(std::string str) {
    std::vector<uint8_t> ret(str.length(), 0);
    memcpy(&ret[0], &str[0], str.length());
    return ret;
}

uint16_t in_cksum (u_int16_t * addr, int len)  
{  
    int nleft = len;  
    uint32_t sum = 0;  
    uint16_t *w = addr;  
    uint16_t answer = 0;  
  
    /* 
    * Our algorithm is simple, using a 32 bit accumulator (sum), we add 
    * sequential 16 bit words to it, and at the end, fold back all the 
    * carry bits from the top 16 bits into the lower 16 bits. 
    */  
    while (nleft > 1) {  
        sum += *w++;  
        nleft -= 2;  
    }  
    /* mop up an odd byte, if necessary */  
    if (nleft == 1) {  
        * (unsigned char *) (&answer) = * (unsigned char *) w;  
        sum += answer;  
    }  
  
    /* add back carry outs from top 16 bits to low 16 bits */  
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */  
    sum += (sum >> 16);     /* add carry */  
    answer = ~sum;     /* truncate to 16 bits */  
    return (answer);  
}