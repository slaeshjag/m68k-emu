#ifndef __MMU_H__
#define __MMU_H__
#include <stdint.h>

void mmu_init();

struct MmuRegTranslationControl {
	uint32_t table_indices_d : 4;
	uint32_t table_indices_c : 4;
	uint32_t table_indices_b : 4;
	uint32_t table_indices_a : 4;
	uint32_t initial_shift : 4;
	uint32_t page_size : 4;
	uint32_t function_code_lookup : 1;
	uint32_t supervisor_root_pointer : 1;
	uint32_t : 5;
	uint32_t enable : 1;
};

struct MmuRegTransparentTranslation {
	uint32_t function_code_mask : 3;
	uint32_t : 1;
	uint32_t function_code_base : 3;
	uint32_t : 1;
	uint32_t read_write_mask : 1;
	uint32_t read_write : 1;
	uint32_t cache_inhibit : 1;
	uint32_t : 4;
	uint32_t enable : 1;
	uint32_t logical_address_mask : 8;
	uint32_t logical_address_base : 8;
};

struct MmuRegStatus {
	uint16_t number_of_levels : 3;
	uint16_t : 3;
	uint16_t transparent : 1;
	uint16_t : 2;
	uint16_t modified : 1;
	uint16_t invalid : 1;
	uint16_t write_protected : 1;
	uint16_t : 1;
	uint16_t supervisor_only : 1;
	uint16_t limit_violation : 1;
	uint16_t bus_error : 1;
};

struct MmuRegRootPointer {
	uint64_t unused : 4;
	uint64_t table_address : 28;
	uint64_t descriptor_type : 2;
	uint64_t : 14;
	uint64_t limit : 15;
	uint64_t lu : 1;
};

struct MmuDescriptorTableShort {
	uint32_t descriptor_type : 2;
	uint32_t write_protected : 1;
	uint32_t used : 1;
	uint32_t table_address : 28;
};

struct MmuDescriptorTableLong {
	uint64_t : 4;
	uint64_t table_address : 28;
	uint64_t descriptor_type : 2;
	uint64_t write_protected : 1;
	uint64_t used : 1;
	uint64_t : 4;
	uint64_t supervisor_only : 1;
	uint64_t : 7;
	uint64_t limit : 15;
	uint64_t lu : 1;
};

struct MmuDescriptorEarlyTerminationShort {
	uint32_t descriptor_type : 2;
	uint32_t write_protected : 1;
	uint32_t used : 1;
	uint32_t modified : 1;
	uint32_t : 1;
	uint32_t cache_inhibit : 1;
	uint32_t : 1;
	uint32_t page_address : 24;
};

struct MmuDescriptorEarlyTerminationLong {
	uint64_t : 8;
	uint64_t page_address : 24;
	uint64_t descriptor_type : 2;
	uint64_t write_protected : 1;
	uint64_t used : 1;
	uint64_t modified : 1;
	uint64_t : 1;
	uint64_t cache_inhibit : 1;
	uint64_t : 1;
	uint64_t supervisor_only : 1;
	uint64_t : 7;
	uint64_t limit : 15;
	uint64_t lu : 1;
};

struct MmuDescriptorPageShort {
	uint32_t descriptor_type : 2;
	uint32_t write_protected : 1;
	uint32_t used : 1;
	uint32_t modified : 1;
	uint32_t : 1;
	uint32_t cache_inhibit : 1;
	uint32_t : 1;
	uint32_t page_address : 24;
};

struct MmuDescriptorPageLong {
	uint64_t : 8;
	uint64_t page_address : 24;
	uint64_t descriptor_type : 2;
	uint64_t write_protected : 1;
	uint64_t used : 1;
	uint64_t modified : 1;
	uint64_t : 1;
	uint64_t cache_inhibit : 1;
	uint64_t : 1;
	uint64_t supervisor_only : 1;
	uint64_t : 7;
	uint64_t : 16;
};

struct MmuDescriptorInvalidShort {
	uint32_t descriptor_type : 2;
	uint32_t : 30;
};

struct MmuDescriptorInvalidLong {
	uint64_t : 32;
	uint64_t descriptor_type : 2;
	uint64_t : 30;
};

struct MmuDescriptorIndirectShort {
	uint32_t descriptor_type : 2;
	uint32_t descriptor_address : 30;
};

struct MmuDescriptorIndirectLong {
	uint64_t : 2;
	uint32_t descriptor_address : 30;
	uint64_t descriptor_type : 2;
	uint64_t : 30;
};

typedef union MmuDescriptorShort MmuDescriptorShort;
union MmuDescriptorShort {
	uint32_t whole;
	struct MmuDescriptorInvalidShort invalid;
	struct MmuDescriptorPageShort page;
	struct MmuDescriptorTableShort table;
	struct MmuDescriptorIndirectShort indirect;
};

typedef union MmuDescriptorLong MmuDescriptorLong;
union MmuDescriptorLong {
	uint64_t whole;
	struct MmuDescriptorInvalidLong invalid;
	struct MmuDescriptorPageLong page;
	struct MmuDescriptorTableLong table;
	struct MmuDescriptorIndirectLong indirect;
};

typedef enum MmuDescriptorType MmuDescriptorType;
enum MmuDescriptorType {
	MMU_DESCRIPTOR_TYPE_INVALID,
	MMU_DESCRIPTOR_TYPE_PAGE,
	MMU_DESCRIPTOR_TYPE_TABLE_SHORT,
	MMU_DESCRIPTOR_TYPE_TABLE_LONG,
};

#endif