#ifndef ESPBOARD_H
#define ESPBOARD_H

#ifdef FAKE_ARDUINO
    #include <chrono>
    #include <map>
    #include <stddef.h>
    using namespace std::chrono;

    typedef long unsigned int asize_t;
    auto processStartTime = std::chrono::high_resolution_clock::now();


    size_t maxHeap=50000;  // assume max of 50000 bytes
    size_t allocatedHeap=0;
    std::map<void*,size_t> mallocSizeMap;

/*
    void* operator new(size_t size) {
        allocatedHeap += size;
        void* block = malloc(size);
        mallocSizeMap[block] = size;
        return block;
    }

    void operator delete(void*p) {
        std::map<void*,size_t>::iterator f = mallocSizeMap.find(p);
        if (f != mallocSizeMap.end()) {
            allocatedHeap -= f->second;
            mallocSizeMap.erase(f);
        }
        free(p);
    }
*/


    class EspBoardClass {
        public:
            long getFreeContStack() {
                return 0;
            }

            long getFreeHeap() { 
                return 0;
            }

            long getMaxFreeBlockSize() {
                return 0;
            }

            long getHeapFragmentation() {
                return 0;
            }

            long currentMsecs() {
                auto now = std::chrono::high_resolution_clock::now();
                duration<double> sinceStart = now-processStartTime;
                long msecs = (long)sinceStart.count();
                printf("ms: %d",msecs);
                return msecs;
            }

            void delayMsecs(size_t msecs) {

            }

    }; 
    EspBoardClass EspBoard;
#else 
    #include <esp.h>
    class EspBoardClass: public EspClass {
        public:
            long currentMsecs() {
                return millis();
            }        

            void delayMsecs(size_t msecs) {
                delay(msecs);
            }

    };
    EspBoardClass EspBoard;
#endif

#endif