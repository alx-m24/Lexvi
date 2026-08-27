#pragma once

#include "kernel/acpi/pci/pci.hpp"
#include "kernel/register/register.hpp"

namespace kernel {
    class DeviceID {
        public:
            using FIELD = Field<uint16_t, uint32_t, 16, 31>;
            using TYPE = FIELD::TYPE;

        private:
            TYPE m_raw{};

        public:
            DeviceID() = default;
            DeviceID(TYPE val) : m_raw(val) {}
            DeviceID& operator=(TYPE val) {
                m_raw = val;
                return *this;
            }

            uint16_t getDeviceID() const {
                return m_raw;
            }
    };

    class VendorID {
        public:
            using FIELD = Field<uint16_t, uint32_t,  0, 15>;
            using TYPE = FIELD::TYPE;

        private:
            TYPE m_raw{};

        public:
            VendorID() = default;
            VendorID(TYPE val) : m_raw(val) {}
            VendorID& operator=(TYPE val) {
                m_raw = val;
                return *this;
            }

            const char* getVendorName() const {
                if (m_raw == 0x8086) return "Intel";
                return "Unknown";
            }
    };


    struct ESPI_DID_VID : 
        public ReadOnlyRegister<uint32_t,
                                getInvalidPCIRegisterState<uint32_t>(),
                                DeviceID::FIELD,
                                VendorID::FIELD> {
        ESPI_DID_VID() = default;
        ESPI_DID_VID(uint32_t val) : ReadOnlyRegister(val) {}

        static constexpr PCIConfigAddress getPCIConfigAddress() {
            return PCIConfigAddress{ 0, 31, 0, 0x00 };
        }
    };

    inline void getVendorDeviceID(VendorID* out_vendorID, DeviceID* out_deviceID) {
        ESPI_DID_VID espi_did_vid = pciConfigRead32<ESPI_DID_VID::getPCIConfigAddress()>();
        *out_vendorID = espi_did_vid.get<VendorID::FIELD>();
        *out_deviceID = espi_did_vid.get<DeviceID::FIELD>();
    }
}
