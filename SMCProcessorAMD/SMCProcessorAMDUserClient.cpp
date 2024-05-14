#include "SMCProcessorAMDUserClient.hpp"

// 定义元类和结构体
OSDefineMetaClassAndStructors(SMCProcessorAMDUserClient, IOUserClient);

// 开始服务
bool SMCProcessorAMDUserClient::start(IOService *provider){

    IOLog("SMCProcessorAMDUserClient::start\n");

    bool success = IOService::start(provider);

    // 如果成功，将提供者转换为SMCProcessorAMD类型
    if(success){
        fProvider = OSDynamicCast(SMCProcessorAMD, provider);
    }

    return success;
}

// 停止服务
void SMCProcessorAMDUserClient::stop(IOService *provider){
    IOLog("SMCProcessorAMDUserClient::stop\n");

    // 将提供者设置为null
    fProvider = nullptr;
    IOService::stop(provider);
}

// 两数相乘
uint64_t multiply_two_numbers(uint64_t number_one, uint64_t number_two){
    uint64_t number_three = 0;
    for(uint32_t i = 0; i < number_two; i++){
        number_three = number_three + number_one;
    }
    return number_three;
}

// 外部方法
IOReturn SMCProcessorAMDUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
                                                   IOExternalMethodDispatch *dispatch, OSObject *target, void *reference){

    IOLog("SMCProcessorAMDUserClient::externalMethod selector:%d\n", selector);

    // 根据选择器选择执行的方法
    switch (selector) {
        case 0: {
            // 两数相乘
            uint64_t r = multiply_two_numbers(arguments->scalarInput[0], arguments->scalarInput[1]);
            arguments->scalarOutput[0] = r;
            arguments->scalarOutputCount = 1;

            IOLog("SMCProcessorAMDUserClient::multiply_two_numbers r:%llu\n", r);

            break;
        }
        case 1: {
            // 读取msr
            uint32_t msr_addr = (uint32_t)(arguments->scalarInput[0]);
            uint64_t msr_value_buf = 0;
            bool err = !fProvider->read_msr(msr_addr, &msr_value_buf);
            if(err){
                IOLog("SMCProcessorAMDUserClient::read_msr: failed at address %u\n", msr_addr);
            } else {
                arguments->scalarOutput[0] = msr_value_buf;
                arguments->scalarOutputCount = 1;
            }

            break;
        }

        case 2: {
            // 获取物理核心数量
            uint32_t numPhyCores = fProvider->totalNumberOfPhysicalCores;

            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = numPhyCores;

            arguments->structureOutputSize = numPhyCores * sizeof(uint64_t);

            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            for(uint32_t i = 0; i < numPhyCores; i++){
                dataOut[i] = fProvider->MSR_HARDWARE_PSTATE_STATUS_perCore[i];
            }

            break;
        }

        case 3: {
            // 获取包温度
            arguments->scalarOutputCount = 0;

            arguments->structureOutputSize = 1 * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;
            dataOut[0] = fProvider->PACKAGE_TEMPERATURE_perPackage[0];
            break;
        }

        default: {
            IOLog("SMCProcessorAMDUserClient::externalMethod: invalid method.\n");
            break;
        }
    }

    return kIOReturnSuccess;
}