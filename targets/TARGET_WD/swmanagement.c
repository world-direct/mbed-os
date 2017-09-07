#include "swmanagement.h"
#include "blsrv.h"

static char m_null_termination;
static size_t m_download_offset;

static inline void m_set_valid_md(intptr_t mdroot, swmanagement_image_information * info){
	info->image_total_length = *((size_t*)(mdroot + 0x04));
	info->image_application_name = (const char *)(mdroot + 0x08);
	info->image_application_version = (const char *)(mdroot + 0x28);
	info->image_cpu_id_mask = *((uint32_t*)(mdroot + 0x48));
	info->image_cpu_id = *((uint32_t*)(mdroot + 0x4C));
}

static inline m_set_invalid_md(swmanagement_image_information * info){
	info->image_total_length = 0;
	info->image_application_name = &m_null_termination;
	info->image_application_version = &m_null_termination;
}

static inline void m_set_md(intptr_t mdroot, swmanagement_image_information * info){
	if (info->image_validation_result == NoMetadata || 
		info->image_validation_result == InvalidMetadata) {
		m_set_invalid_md(info);
	}
	else{
		m_set_valid_md(mdroot, info);
	}
}

void swmanagement_get_status(swmanagement_status * status)
{
	struct blsrv_desc desc;

	// validate boot image
	//////////////////////////////////////////////////////////////////////////
	desc.operation = blsrv_validate_boot_image;
	blsrv_call(&desc);
	status->boot_image_information.image_validation_result = desc.args.validate_image.validation_result;
	m_set_md(desc.args.validate_image.metadata_ptr, &status->boot_image_information);

	// validate update image
	//////////////////////////////////////////////////////////////////////////
	desc.operation = blsrv_validate_update_image;
	blsrv_call(&desc);
	status->update_image_information.image_validation_result = desc.args.validate_image.validation_result;
	m_set_md(desc.args.validate_image.metadata_ptr, &status->update_image_information);

	status->update_status = desc.args.validate_image.command_word;
}

void swmanagement_prepare_new_download(void)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_erase_update_region;

	blsrv_call(&desc);
	
	m_download_offset = 0;
}

void swmangement_append_download_data(const void * buffer, size_t buffer_size)
{
	intptr_t src = (intptr_t)buffer;

	struct blsrv_desc desc;
	desc.operation = blsrv_write_update_region;
	desc.args.write_update_region.src = src;
	desc.args.write_update_region.offset = m_download_offset;
	desc.args.write_update_region.size = buffer_size;

	m_download_offset += buffer_size;

	blsrv_call(&desc);

}

image_validation_result swmanagement_update_apply(void)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_apply_update;
	blsrv_call(&desc);

	return desc.args.apply_update.validation_result;
}