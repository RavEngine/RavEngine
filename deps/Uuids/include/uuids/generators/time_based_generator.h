#ifndef UUIDS_TIME_BASED_GENERATOR_HPP
#define UUIDS_TIME_BASED_GENERATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "generator_base.h"

#include <chrono>
#include <random>

#ifdef __unix__
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#else // windows
#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")
#endif

namespace uuids
{
    struct time_based_generator : public generator_base
    {
    public:
        time_based_generator()
        {
            _rnd_engine.seed(std::chrono::system_clock::now().time_since_epoch().count());
        };
        ~time_based_generator() {}

        uint16_t get_version()
        {
            return 0x1000;
        }

        uint64_t generate_timestamp()
        {
            auto base_time = std::chrono::high_resolution_clock::from_time_t(_base_date);
            auto today_midnight = std::chrono::high_resolution_clock::now();
            auto diff_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(today_midnight - base_time).count();
            return static_cast<uint64_t>(diff_ns / 100) + _base_date_cnt_100ns;
        }

        uint8_t get_variant()
        {
            return 0x80;
        }

        uint16_t generate_clock_sequence()
        {
            std::time_t now = std::time(NULL);

            // if clock seq not yet generated OR clock has gone backward, generate new
            if (_last_used_time == -1 || now < _last_used_time)
            {
                _last_clock_seq = _rnd_uint16_distribution(_rnd_engine);
            }
            else // increase clock seq
            {
                _last_clock_seq++; // not caring about overflow
            }

            _last_used_time = now;
            return _last_clock_seq;
        }

        uint8_t *get_node()
        {
            bool ok;
            unsigned char *mac = _get_MAC_address(ok);

            // create random node if not ok
            static uint8_t node[6];
            if (ok)
            {
                for (uint8_t i = 0; i < 6; ++i)
                    node[i] = static_cast<uint8_t>(mac[i]);
            }
            else
            {
                for (uint8_t i = 0; i < 6; ++i)
                    node[i] = static_cast<uint8_t>(_rnd_uint16_distribution(_rnd_engine));
            }

            // save this node as last one
            bool different_from_previous = false;
            for (uint8_t i = 0; i < 6; ++i)
            {
                if (node[i] != _last_node_id[i])
                    different_from_previous = true;
                _last_node_id[i] = node[i];
            }

            // if this node differs from previous, force clock seq to be randomized
            if (different_from_previous)
                _last_used_time = -1;

            return node;
        }

    private:
        time_based_generator(const time_based_generator &) = delete;
        unsigned char *_get_MAC_address(bool &ok)
        {
            ok = false;
            int success = 0;
            static unsigned char mac_address[6];

#ifdef __unix__
            // taken from: https://stackoverflow.com/a/1779758
            struct ifreq ifr;
            struct ifconf ifc;
            char buf[1024];

            int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
            if (sock == -1)
                return nullptr;

            ifc.ifc_len = sizeof(buf);
            ifc.ifc_buf = buf;
            if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
            {
                close(sock);
                return nullptr;
            }

            struct ifreq *it = ifc.ifc_req;
            const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));
            for (; it != end; ++it)
            {
                strcpy(ifr.ifr_name, it->ifr_name);

                if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
                {
                    if (!(ifr.ifr_flags & IFF_LOOPBACK))
                    {
                        if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
                        {
                            success = 1;
                            break;
                        }
                    }
                }
            }

            close(sock);
            if (success)
                memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);

            ok = bool(success);
            return mac_address;
#else // windows
            PIP_ADAPTER_INFO AdapterInfo;
            DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

            // taken from: https://stackoverflow.com/a/13688254

            AdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
            if (AdapterInfo == NULL)
            {
                std::cout << "Error allocating memory needed to call GetAdaptersinfo\n";
                return nullptr;
            }

            // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
            if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW)
            {
                free(AdapterInfo);
                AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
                if (AdapterInfo == NULL)
                {
                    std::cout << "Error allocating memory needed to call GetAdaptersinfo\n";
                    return nullptr;
                }
            }

            if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
            {
                // Contains pointer to current adapter info
                PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
                do
                {
                    mac_address[0] = pAdapterInfo->Address[0];
                    mac_address[1] = pAdapterInfo->Address[1];
                    mac_address[2] = pAdapterInfo->Address[2];
                    mac_address[3] = pAdapterInfo->Address[3];
                    mac_address[4] = pAdapterInfo->Address[4];
                    mac_address[5] = pAdapterInfo->Address[5];
                    success = 1;
                    break;

                    pAdapterInfo = pAdapterInfo->Next;
                } while (pAdapterInfo);
            }

            ok = bool(success);
            free(AdapterInfo);
            return mac_address;
#endif
        }

    private:
        inline static const constexpr std::time_t _base_date = 946684800;                 // 01/01/2000, 00:00:00
        inline static const constexpr uint64_t _base_date_cnt_100ns = 131659776000000000; // computed a priori, from 15/10/1582 00:00:00 to base date
        inline static std::time_t _last_used_time = -1;                                   // default, forces to generate new cloc seq
        inline static uint16_t _last_clock_seq = 0;                                       // last generated clock seq
        inline static uint8_t _last_node_id[6] = {0, 0, 0, 0, 0, 0};                      // last node id read from MAC (if available)
        inline static std::default_random_engine _rnd_engine = std::default_random_engine();
        inline static std::uniform_int_distribution<uint16_t> _rnd_uint16_distribution = std::uniform_int_distribution<uint16_t>(0, UINT16_MAX);
    };
} // namespace uuids

#endif // UUIDS_TIME_BASED_GENERATOR_HPP
