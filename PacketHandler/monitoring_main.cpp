#include <iostream>
#include <vector>
#include <string>
#include <pcap/pcap.h>
#include "monitoring/packet_processor.h"
#include "monitoring/commutation_table.h"
#include "utils/network_utils.h"
#include "utils/config_parser.h"


int main(int argc, char* argv[]) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* alldevs;

    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        std::cerr << "Error finding devices: " << errbuf << std::endl;
        return 1;
    }

    if (!alldevs) {
        std::cerr << "No network interfaces found!" << std::endl;
        return 1;
    }

    int i = 0;
    for (pcap_if_t* d = alldevs; d; d = d->next) {
        std::cout << ++i << ". " << d->name;
        if (d->description)
            std::cout << " (" << d->description << ")";
        std::cout << std::endl;
    }

    std::string selectedInterface;
    if (argc > 1) {
        selectedInterface = argv[1];
    } else {
        std::cout << "Enter the interface number: ";
        int inum;
        std::cin >> inum;

        pcap_if_t* d;
        for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);
        selectedInterface = d->name;
    }

    pcap_t* handle = pcap_open_live(selectedInterface.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (!handle) {
        std::cerr << "Couldn't open interface " << selectedInterface << ": " << errbuf << std::endl;
        pcap_freealldevs(alldevs);
        return 1;
    }

    struct bpf_program fp;
    std::string filter = "arp or ip";
    if (pcap_compile(handle, &fp, filter.c_str(), 0, PCAP_NETMASK_UNKNOWN) == -1) {
        std::cerr << "Couldn't parse filter " << filter << ": " << pcap_geterr(handle) << std::endl;
        pcap_close(handle);
        pcap_freealldevs(alldevs);
        return 1;
    }

    if (pcap_setfilter(handle, &fp) == -1) {
        std::cerr << "Couldn't install filter " << filter << ": " << pcap_geterr(handle) << std::endl;
        pcap_freecode(&fp);
        pcap_close(handle);
        pcap_freealldevs(alldevs);
        return 1;
    }

    pcap_freecode(&fp);
    pcap_freealldevs(alldevs);

    std::cout << "Starting packet monitoring on interface " << selectedInterface << "..." << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    pcap_loop(handle, 0, PacketProcessor::handler, nullptr);

    pcap_close(handle);
    return 0;
}