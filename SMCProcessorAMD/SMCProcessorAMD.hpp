#ifndef SMCProcessorAMD_h
#define SMCProcessorAMD_h


#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>

#include <i386/proc_reg.h>
#include <libkern/libkern.h>


#include <Headers/kern_util.hpp>
#include <Headers/kern_cpu.hpp>
#include <Headers/kern_time.hpp>

#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <VirtualSMCSDK/AppleSmc.h>

#include "KeyImplementations.hpp"


extern "C" {
    int cpu_number(void);
    void mp_rendezvous_no_intrs(void (*action_func)(void *), void *arg);

    void
    mp_rendezvous(void (*setup_func)(void *),
                  void (*action_func)(void *),
                  void (*teardown_func)(void *),
                  void *arg);

//    int wrmsr_carefully(uint32_t msr, uint64_t val);
};


/**
 * Offset table: https://github.com/torvalds/linux/blob/master/drivers/hwmon/k10temp.c#L78
 */
typedef struct tctl_offset {
    uint8_t model;
    char const *id;
    int offset;
} TempOffset;


class SMCProcessorAMD : public IOService {
    OSDeclareDefaultStructors(SMCProcessorAMD)
    
    /**
     *  VirtualSMC service registration notifier
     */
    IONotifier *vsmcNotifier {nullptr};
    
    static bool vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier);
    
    /**
     *  Registered plugin instance
     */
    VirtualSMCAPI::Plugin vsmcPlugin {
        xStringify(PRODUCT_NAME),
        parseModuleVersion(xStringify(MODULE_VERSION)),
        VirtualSMCAPI::Version,
    };
    
    
    /**
     *  MSRs supported by AMD 17h CPU from:
     *  https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/master/LibreHardwareMonitorLib/Hardware/Cpu/Amd17Cpu.cs
     */
    static constexpr uint32_t kCOFVID_STATUS = 0xC0010071;
    static constexpr uint32_t k17H_M01H_SVI = 0x0005A000;
    static constexpr uint32_t kF17H_M01H_THM_TCON_CUR_TMP = 0x00059800;
    static constexpr uint32_t kF17H_M70H_CCD1_TEMP = 0x00059954;
    static constexpr uint32_t kF17H_TEMP_OFFSET_FLAG = 0x80000;
    static constexpr uint8_t kFAMILY_17H_PCI_CONTROL_REGISTER = 0x60;
    static constexpr uint32_t kHWCR = 0xC0010015;
    static constexpr uint32_t kMSR_CORE_ENERGY_STAT = 0xC001029A;
    static constexpr uint32_t kMSR_HARDWARE_PSTATE_STATUS = 0xC0010293;
    static constexpr uint32_t kMSR_PKG_ENERGY_STAT = 0xC001029B;
    static constexpr uint32_t kMSR_PSTATE_0 = 0xC0010064;
    static constexpr uint32_t kMSR_PWR_UNIT = 0xC0010299;
    static constexpr uint32_t kPERF_CTL_0 = 0xC0010000;
    static constexpr uint32_t kPERF_CTR_0 = 0xC0010004;

    
    /**
     *  Key name index mapping
     */
    static constexpr size_t MaxIndexCount = sizeof("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") - 1;
    static constexpr const char *KeyIndexes = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    
    /**
     *  Supported SMC keys
     */
    static constexpr SMC_KEY KeyPCPR = SMC_MAKE_IDENTIFIER('P','C','P','R');
    static constexpr SMC_KEY KeyPCPT = SMC_MAKE_IDENTIFIER('P','C','P','T');
    static constexpr SMC_KEY KeyPCTR = SMC_MAKE_IDENTIFIER('P','C','T','R');
    static constexpr SMC_KEY KeyTCxD(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'D'); }
    static constexpr SMC_KEY KeyTCxE(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'E'); }
    static constexpr SMC_KEY KeyTCxF(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'F'); }
    static constexpr SMC_KEY KeyTCxG(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'G'); }
    static constexpr SMC_KEY KeyTCxJ(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'J'); }
    static constexpr SMC_KEY KeyTCxH(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'H'); }
    static constexpr SMC_KEY KeyTCxP(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'P'); }
    static constexpr SMC_KEY KeyTCxT(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'T'); }
    static constexpr SMC_KEY KeyTCxp(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'p'); }
    static constexpr SMC_KEY KeyPCxc(size_t i) { return SMC_MAKE_IDENTIFIER('P','C',KeyIndexes[i],'p'); }
    static constexpr SMC_KEY KeyTCxc(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'c'); }
    static constexpr SMC_KEY KeyTCxC(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'C'); }
	static constexpr SMC_KEY KeyVCxC(size_t i) { return SMC_MAKE_IDENTIFIER('V','C',KeyIndexes[i],'C'); }

    static constexpr SMC_KEY KeyTGxP(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'P'); }
    static constexpr SMC_KEY KeyTGxD(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'D'); }
    static constexpr SMC_KEY KeyTGxp(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'p'); }
    static constexpr SMC_KEY KeyTGxd(size_t i) { return SMC_MAKE_IDENTIFIER('T', 'G', KeyIndexes[i], 'd'); }
    static constexpr SMC_KEY KeyTGDD = SMC_MAKE_IDENTIFIER('T', 'G', 'D', 'D');
    static constexpr SMC_KEY KeyTCGC = SMC_MAKE_IDENTIFIER('T', 'C', 'G', 'C');
    static constexpr SMC_KEY KeyVD0R = SMC_MAKE_IDENTIFIER('V', 'D', '0', 'R');
    static constexpr SMC_KEY KeyVD0R = SMC_MAKE_IDENTIFIER('I', 'D', '0', 'R');
public:
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    
    /**
     *  A simple wrapper for the kernel function readmsr_carefully.
     */
    bool read_msr(uint32_t addr, uint64_t *value);
    bool write_msr(uint32_t addr, uint64_t value);
    void setCPBState(bool enabled);
    bool getCPBState();
      
    void updateClockSpeed();
    void updatePackageTemp();
    void updatePackageEnergy();
    
    uint32_t totalNumberOfPhysicalCores;
    uint32_t totalNumberOfLogicalCores;
    
    uint8_t cpuFamily;
    uint8_t cpuModel;
    uint8_t cpuSupportedByCurrentVersion;
    
    //Cache size in KB
    uint32_t cpuCacheL1_perCore;
    uint32_t cpuCacheL2_perCore;
    uint32_t cpuCacheL3;
    
    char boardVender[64]{};
    char boardName[64]{};
    bool boardInfoValid;
    
    /**
     *  Hard allocate space for cached readings.
     */
    uint64_t MSR_HARDWARE_PSTATE_STATUS_perCore[24] {};
    float PACKAGE_TEMPERATURE_perPackage[CPUInfo::MaxCpus];
    
    bool cpbSupported;
    
    uint64_t lastUpdateTime;
    uint64_t lastUpdateEnergyValue;
    
    double uniPackageEnergy;
    
    
private:
    
    IOWorkLoop *workLoop;
    IOTimerEventSource *timerEventSource;
    
    CPUInfo::CpuTopology cpuTopology {};
    
    IOPCIDevice *fIOPCIDevice;
    
    float tempOffset = 0;
    
    int (*wrmsr_carefully)(uint32_t, uint32_t, uint32_t) {nullptr};
    bool setupKeysVsmc();
    bool getPCIService();
    
};
#endif
