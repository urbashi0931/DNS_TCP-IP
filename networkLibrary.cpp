#include <Windows.h>
#include <iostream>
#include <vector>

// Function to set the DNS servers
bool SetDnsServers(const std::vector<std::string>& dnsServers) {
    bool success = true;

    // Retrieve the network adapter information
    IP_ADAPTER_ADDRESSES* adapterList = nullptr;
    ULONG bufferSize = 0;

    DWORD result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &bufferSize);
    if (result == ERROR_BUFFER_OVERFLOW) {
        adapterList = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(new char[bufferSize]);
        result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, adapterList, &bufferSize);

        if (result == ERROR_SUCCESS) {
            // Iterate through the adapters
            IP_ADAPTER_ADDRESSES* adapter = adapterList;
            while (adapter) {
                // Skip loopback and tunnel adapters
                if (adapter->IfType == IF_TYPE_LOOPBACK || adapter->IfType == IF_TYPE_TUNNEL) {
                    adapter = adapter->Next;
                    continue;
                }

                // Configure the DNS servers for each adapter
                IP_PER_ADAPTER_INFO* adapterInfo = adapter->FirstPerAdapterInfo;
                while (adapterInfo) {
                    if (adapterInfo->AutoconfigActive || adapterInfo->DnsEnabled) {
                        // Configure the DNS servers
                        IP_ADDR_STRING* dnsServer = &(adapterInfo->DnsServerList);
                        dnsServer->IpAddress.String = const_cast<char*>(dnsServers[0].c_str());
                        dnsServer->Next = new IP_ADDR_STRING;

                        IP_ADDR_STRING* nextDnsServer = dnsServer->Next;
                        nextDnsServer->IpAddress.String = const_cast<char*>(dnsServers[1].c_str());

                        break;
                    }

                    adapterInfo = adapterInfo->Next;
                }

                adapter = adapter->Next;
            }
        }
        else {
            std::cerr << "Failed to retrieve adapter addresses." << std::endl;
            success = false;
        }

        delete[] reinterpret_cast<char*>(adapterList);
    }
    else {
        std::cerr << "Failed to retrieve adapter addresses size." << std::endl;
        success = false;
    }

    return success;
}

// Function to set the IP address and subnet mask
bool SetIpAddress(const std::string& ipAddress, const std::string& subnetMask) {
    bool success = true;

    // Retrieve the network adapter information
    IP_ADAPTER_INFO* adapterList = nullptr;
    ULONG bufferSize = 0;

    if (GetAdaptersInfo(nullptr, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
        adapterList = reinterpret_cast<IP_ADAPTER_INFO*>(new char[bufferSize]);
        if (GetAdaptersInfo(adapterList, &bufferSize) == NO_ERROR) {
            // Find the first non-loopback adapter
            IP_ADAPTER_INFO* adapter = adapterList;
            while (adapter) {
                if (adapter->Type != MIB_IF_TYPE_LOOPBACK) {
                    break;
                }
                adapter = adapter->Next;
            }

            if (adapter) {
                // Set the IP address and subnet mask
                IP_ADDR_STRING* ipAddr = &(adapter->IpAddressList);
               
