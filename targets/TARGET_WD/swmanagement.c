#include "swmanagement.h"
#include "blsrv.h"


static size_t m_download_offset;

static inline void m_set_md(intptr_t mdroot, swmanagement_image_information * info){
	info->image_total_length = *((size_t*)(mdroot + 0x04));
	info->image_application_name = (const char *)(mdroot + 0x08);
	info->image_application_version = (const char *)(mdroot + 0x28);
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

	desc.operation = blsrv_get_update_status;
	blsrv_call(&desc);
	status->update_status = desc.args.get_update_status.update_status;
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

void swmanagement_update_apply(void)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_apply_update_with_reset;
	blsrv_call(&desc);
}